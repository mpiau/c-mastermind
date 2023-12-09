#include "console.h"

#include <fcntl.h>
#include <stdio.h>
#include <windows.h>
#include <stdatomic.h>

struct Console
{
    u32 oldConsoleMode;
    HANDLE inputHandle;
    HANDLE outputHandle;
    HANDLE backOutputHandle;
    bool swapBuffer;
};


static atomic_bool s_isInit = false;
static struct Console s_console = (struct Console) {};


static BOOL console_ctrl_handler( DWORD const ctrlType )
{
    /*switch ( ctrlType )
    {
	case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
	default: 	assert( false ); // Shouldn't happen
    }*/

	// Call cleanup code.

    console_global_uninit();
	exit( 0 );
}


static bool disable_inputs_display( void )
{
	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	if ( !hStdin ) { return false; }

	GetConsoleMode( hStdin, (LPDWORD)&s_console.oldConsoleMode );

	DWORD const consoleModeIn  = s_console.oldConsoleMode & ~( ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT );
	return SetConsoleMode( hStdin,  consoleModeIn );
}


static COORD screen_get_size( HANDLE const screen )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( screen, &info );

    SHORT const newScreenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    SHORT const newscreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    return (COORD) { .X = newScreenWidth, .Y = newscreenHeight };
}

// ////////////////////////////////////////////////////////////////////////////////////////////

bool console_global_init( char const *optTitle )
{
    bool const expectedValue = false;
    if ( !atomic_compare_exchange_strong( &s_isInit, &expectedValue, true ) )
    {
        fprintf( stderr, "[ERROR]: We shouldn't initialize the Console multiple times !\n" );
        return false;
    }

    // Detach from the existing console if exists
    // We don't want to run on the same console as the one that started the executable.
    FreeConsole();

    // Allocate a new console directly afterwards, in order to have a dedicated console for the program.
	AllocConsole();

   	// https://stackoverflow.com/questions/51726140/console-with-enable-line-input-doesnt-pass-r
	fflush( stdin ); // Important to flush before modifying the mode.
	_setmode( _fileno( stdin ), _O_BINARY );

    disable_inputs_display();

    if ( optTitle != NULL )
    {
        SetConsoleTitleA( optTitle );
    }

   	SetConsoleCtrlHandler( console_ctrl_handler, TRUE );

    s_console.swapBuffer = false;
    s_console.inputHandle = GetStdHandle( STD_INPUT_HANDLE );
    s_console.outputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    s_console.backOutputHandle = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL );

    console_cursor_hide();

    // ///////
    COORD screenSize = screen_get_size( s_console.outputHandle );

    SetConsoleScreenBufferSize( s_console.outputHandle, screenSize );
    SetConsoleScreenBufferSize( s_console.backOutputHandle, screenSize );

    // GetLargestConsoleWindowSize
    // GetConsoleWindow
    // SetConsoleWindowInfo(s_console.backOutputHandle, TRUE, );

    // https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
    // https://learn.microsoft.com/en-us/windows/console/classic-vs-vt 

    return true;
}


void console_global_uninit( void )
{
    if ( !console_is_global_init() )
    {
        fprintf( stderr, "[ERROR]: Trying to uninit the Console, which isn't initialized !\n" );
        return;
    }

    SetConsoleCtrlHandler( console_ctrl_handler, FALSE );
    SetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ),  s_console.oldConsoleMode );

    s_console = (struct Console) {};
    s_console.inputHandle = INVALID_HANDLE_VALUE;
    s_console.outputHandle = INVALID_HANDLE_VALUE;

    atomic_store( &s_isInit, false );
}


bool console_is_global_init( void )
{
    return atomic_load( &s_isInit );
}


// Cursor visibility //


static bool cursor_set_visibility( bool const visible )
{
   	CONSOLE_CURSOR_INFO cursorInfo = {};

	if ( !GetConsoleCursorInfo( s_console.outputHandle, &cursorInfo ) )
	{
		return false;
	}

    cursorInfo.bVisible = visible;

    return SetConsoleCursorInfo( s_console.outputHandle, &cursorInfo );
}


bool console_cursor_show( void )
{
    return cursor_set_visibility( true );
}


bool console_cursor_hide( void )
{
    return cursor_set_visibility( false );
}


bool console_cursor_is_shown( void )
{
    assert( s_isInit );

   	CONSOLE_CURSOR_INFO cursorInfo = {};

	if ( !GetConsoleCursorInfo( s_console.outputHandle, &cursorInfo ) )
	{
		return false;
	}

    return cursorInfo.bVisible;
}

/*
ESC [ <n> A 	CUU 	Cursor Up 	Cursor up by <n>
ESC [ <n> B 	CUD 	Cursor Down 	Cursor down by <n>
ESC [ <n> C 	CUF 	Cursor Forward 	Cursor forward (Right) by <n>
ESC [ <n> D 	CUB 	Cursor Backward 	Cursor backward (Left) by <n>
ESC [ <n> @ 	ICH 	Insert Character 	Insert <n> spaces at the current cursor position, shifting all existing text to the right. Text exiting the screen to the right is removed.
ESC [ <n> P 	DCH 	Delete Character 	Delete <n> characters at the current cursor position, shifting in space characters from the right edge of the screen.
ESC [ <n> X 	ECH 	Erase Character 	Erase <n> characters from the current cursor position by overwriting them with a space character.
ESC [ <n> L 	IL 	Insert Line 	Inserts <n> lines into the buffer at the cursor position. The line the cursor is on, and lines below it, will be shifted downwards.
ESC [ <n> M 	DL 	Delete Line 	Deletes <n> lines from the buffer, starting with the row the cursor is on.
*/

void console_print( char const *const buffer )
{
    if ( !buffer ) return;

    HANDLE bufferToWrite          = s_console.swapBuffer ? s_console.backOutputHandle : s_console.outputHandle;
    HANDLE currentDisplayedBuffer = s_console.swapBuffer ? s_console.outputHandle : s_console.backOutputHandle;

   	CONSOLE_SCREEN_BUFFER_INFO csinfo;
	GetConsoleScreenBufferInfo( currentDisplayedBuffer, &csinfo );

    u32 const lines = csinfo.dwCursorPosition.Y;
    u32 const columns = csinfo.dwCursorPosition.X;

    char buf[128] = {};
    sprintf( buf, "\x1B[%uA\x1B[%uD\x1B[%uM", lines, columns, lines );

    //SetConsoleCursorPosition( bufferToWrite, (COORD){ .X = 1, .Y = 1 } );
    WriteConsole( bufferToWrite, buffer, strlen( buffer ), NULL, NULL);
    SetConsoleActiveScreenBuffer( bufferToWrite );

    WriteConsole( currentDisplayedBuffer, buf, strlen( buf ), NULL, NULL );

    // \x1B[2J 
    // only \x1B[3J when the screen has been resized, it cleans the scroll history !
    // printf( "\x1B[%uA\x1B[%uD\x1B[%uM%s", lines, columns, lines, buffer );
    // printf( "\x1B[%uA\x1B[%uD\x1B[%uM%s", lines, columns, lines, buffer );
    // printf( "\033[1;1H" );
    // printf( "%s", buffer );

    // "\033[H\x1B[2J\x1B[3J" // H -> cursor top, 2J == clean current screen and 3J == clean scroll history

    s_console.swapBuffer = !s_console.swapBuffer;
}
