#include "console.h"

#include <fcntl.h>
#include <stdio.h>
#include <windows.h>
#include <stdatomic.h>
#include <stdarg.h>
#include <wchar.h>

struct Console
{
    HANDLE inputHandle;
    HANDLE outputHandle;
    HANDLE backOutputHandle;
    bool swapBuffer;
};

static u32 s_oldInputMode = 0;
static u32 s_oldOutputMode = 0;
static atomic_bool s_isInit = false;

static utf16 s_screenBuffer[4096] = {};
static u16 s_bufPos = 0;

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


static void update_console_mode( void )
{
	HANDLE handle = console_input_handle();
	GetConsoleMode( handle, (LPDWORD)&s_oldInputMode );

	DWORD newMode = s_oldInputMode & ~( ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT ) | ENABLE_WINDOW_INPUT;
	SetConsoleMode( handle,  newMode );

	handle = console_output_handle();
	GetConsoleMode( handle, (LPDWORD)&s_oldOutputMode );

	newMode = ( s_oldOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN );
	SetConsoleMode( handle, newMode );
}


static void reset_console_mode( void )
{
    SetConsoleMode( console_input_handle(),  s_oldInputMode );
    SetConsoleMode( console_output_handle(), s_oldOutputMode );
}

// ////////////////////////////////////////////////////////////////////////////////////////////

bool console_global_init( char const *optTitle, bool const onDedicatedConsole )
{
    bool const expectedValue = false;
    if ( !atomic_compare_exchange_strong( &s_isInit, &expectedValue, true ) )
    {
        fprintf( stderr, "[ERROR]: We shouldn't initialize the Console multiple times !\n" );
        return false;
    }

    if ( onDedicatedConsole )
    {
        // Detach from the existing console if exists
        // We don't want to run on the same console as the one that started the executable.
        FreeConsole();

        // Allocate a new console directly afterwards, in order to have a dedicated console for the program.
        AllocConsole();
    }

   	// https://stackoverflow.com/questions/51726140/console-with-enable-line-input-doesnt-pass-r
	// fflush( stdin ); // Important to flush before modifying the mode.
	// _setmode( _fileno( stdin ), _O_BINARY );

    if ( optTitle != NULL )
    {
        console_set_title( optTitle );
    }

    update_console_mode();
   	SetConsoleCtrlHandler( console_ctrl_handler, TRUE );
    _setmode( _fileno( stdout ), _O_U16TEXT ); // To correctly print wchar_t

	console_alternate_buffer_enter();
    console_cursor_hide();

    vec2u16 const screenSize = console_screen_get_size( console_output_handle() );
    SetConsoleScreenBufferSize( console_output_handle(), *(COORD *)&screenSize );

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
    reset_console_mode();
    console_cursor_show();
    console_line_drawing_mode_exit();
    console_color_reset();
    console_alternate_buffer_exit();

    atomic_store( &s_isInit, false );
}


bool console_is_global_init( void )
{
    return atomic_load( &s_isInit );
}


bool console_set_title( char const *const title )
{
	if ( !title ) return false;
	if ( strnlen( title, 256 ) >= 255 )
	{
		fprintf( stderr, "Title can't be larger than 255 characters.\n" );
		return false;
	}

	console_draw( L"\x1B]0;%s\007", title );
	return true;
}


HANDLE console_input_handle( void )
{
    return GetStdHandle( STD_INPUT_HANDLE );
}

HANDLE console_output_handle( void )
{
    return GetStdHandle( STD_OUTPUT_HANDLE );
}

// Cursor visibility //

void console_cursor_hide( void )
{
	console_draw( L"\x1B[?25l" );
}

void console_cursor_show( void )
{
	console_draw( L"\x1B[?25h" );
}

void console_cursor_start_blinking( void )
{
	console_draw( L"\x1B[?12h" );
}

void console_cursor_stop_blinking( void )
{
	console_draw( L"\x1B[?12l" );
}

void console_cursor_set_position( short const y, short const x )
{
    console_draw( L"\x1B[%u;%uH", y, x );
}

void console_cursor_move_up_by( short const n )
{
    console_draw( L"\x1B[%uA", n );
}

void console_cursor_move_down_by( short const n )
{
    console_draw( L"\x1B[%uB", n );
}

void console_cursor_move_left_by( short const n )
{
    console_draw( L"\x1B[%uD", n );
}

void console_cursor_move_right_by( short const n )
{
    console_draw( L"\x1B[%uC", n );
}

/*
ESC [ <n> @ 	ICH 	Insert Character 	Insert <n> spaces at the current cursor position, shifting all existing text to the right. Text exiting the screen to the right is removed.
ESC [ <n> P 	DCH 	Delete Character 	Delete <n> characters at the current cursor position, shifting in space characters from the right edge of the screen.
ESC [ <n> X 	ECH 	Erase Character 	Erase <n> characters from the current cursor position by overwriting them with a space character.
ESC [ <n> L 	IL 	Insert Line 	Inserts <n> lines into the buffer at the cursor position. The line the cursor is on, and lines below it, will be shifted downwards.
ESC [ <n> M 	DL 	Delete Line 	Deletes <n> lines from the buffer, starting with the row the cursor is on.
*/

void console_line_drawing_mode_enter( void )
{
	console_draw( L"\x1B(0" );
}

void console_line_drawing_mode_exit( void )
{
	console_draw( L"\x1B(B" );
}


void console_alternate_buffer_enter( void )
{
	console_draw( L"\x1b[?1049h" );
}

void console_alternate_buffer_exit( void )
{
	console_draw( L"\x1b[?1049l" );
}


void console_color_reset( void )
{
    console_draw( L"\x1b[0;0m" );
}

void console_color_bold( void )
{
    console_draw( L"\x1b[1m" );
}

void console_color_no_bold( void )
{
    console_draw( L"\x1b[22m" );
}

void console_color_underline( void )
{
    console_draw( L"\x1b[4m" );
}

void console_color_no_underline( void )
{
    console_draw( L"\x1b[24m" );
}

void console_color_negative( void )
{
    console_draw( L"\x1b[7m" );
}

void console_color_positive( void )
{
    console_draw( L"\x1b[27m" );
}

void console_color( enum ConsoleColorFG const fgColor, enum ConsoleColorBG const bgColor )
{
    console_draw( L"\x1b[%u;%um", fgColor, bgColor );
}

void console_color_fg( enum ConsoleColorFG const fgColor )
{
    console_draw( L"\x1b[%um", fgColor );
}

void console_color_bg( enum ConsoleColorBG const bgColor )
{
    console_draw( L"\x1b[%um", bgColor );
}


vec2u16 console_screen_get_size( HANDLE const handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( handle, &info );

    u16 const newScreenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    u16 const newscreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    return (vec2u16) { .x = newScreenWidth, .y = newscreenHeight };    
}


void console_screen_clear( void )
{
	console_draw( L"\x1b[2J" );
}

// ////////////////////////////////////////////////

void console_setpos( screenpos pos )
{
    console_draw( L"\x1B[%u;%uH", pos.y, pos.x );
}


void console_draw( utf16 const *format, ... )
{
	va_list args;
	va_start( args, format );
	s_bufPos += vsnwprintf( s_screenBuffer + s_bufPos, ARR_COUNT( s_screenBuffer ) - s_bufPos, format, args );
	va_end( args );
}


void console_refresh( void )
{
    if ( s_bufPos > 0 )
    {
        wprintf( s_screenBuffer );
        s_bufPos = 0;
    }
}
