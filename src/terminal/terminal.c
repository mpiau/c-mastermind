#include "terminal/terminal.h"


#include <fcntl.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdarg.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


static u32 s_oldInputMode = 0;
static u32 s_oldOutputMode = 0;
static atomic_bool s_isInit = false;

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

    term_uninit();
	exit( 0 );
}


static void term_enter_alternate_buffer( void )
{
	wprintf( L"\x1b[?1049h" );
}

static void term_exit_alternate_buffer( void )
{
	wprintf( L"\x1b[?1049l" );
}


static void update_console_mode( void )
{
	HANDLE handle = term_input_handle();
	GetConsoleMode( handle, (LPDWORD)&s_oldInputMode );

    // Funny thing, we need ENABLE_EXTENDED_FLAGS alongside ENABLE_MOUSE_INPUT in order to have mouse inputs
    // And this flag just isn't documented on the Windows documentation.
    // See https://stackoverflow.com/questions/37069599/cant-read-mouse-event-use-readconsoleinput-in-c 
	DWORD newMode = ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT;
	SetConsoleMode( handle,  newMode );

	handle = term_output_handle();
	GetConsoleMode( handle, (LPDWORD)&s_oldOutputMode );

	newMode = ( ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN );
	SetConsoleMode( handle, newMode );
}


static void reset_console_mode( void )
{
    SetConsoleMode( term_input_handle(),  s_oldInputMode );
    SetConsoleMode( term_output_handle(), s_oldOutputMode );
}

// ////////////////////////////////////////////////////////////////////////////////////////////

bool term_init( char const *optTitle, bool const onDedicatedConsole )
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
        term_set_title( optTitle );
    }

    update_console_mode();
   	SetConsoleCtrlHandler( console_ctrl_handler, TRUE );
    _setmode( _fileno( stdout ), _O_U16TEXT ); // To correctly print wchar_t

	term_enter_alternate_buffer();
    cursor_hide();

    term_screen_init( term_output_handle() );
    screensize const screenSize = term_size();
    SetConsoleScreenBufferSize( term_output_handle(), *(COORD *)&screenSize );

    return true;
}

void term_uninit( void )
{
    if ( !term_is_init() ) return;

    SetConsoleCtrlHandler( console_ctrl_handler, FALSE );
    reset_console_mode();
    cursor_show();
    wprintf( L"\x1B[0;0m" );
    term_exit_alternate_buffer();

    atomic_store( &s_isInit, false );
}

bool term_is_init( void )
{
    return atomic_load( &s_isInit );
}


bool term_set_title( char const *const title )
{
	if ( !title ) return false;
	if ( strnlen( title, 256 ) >= 255 )
	{
		fprintf( stderr, "Title can't be larger than 255 characters.\n" );
		return false;
	}

	wprintf( L"\x1B]0;%s\007", title );
	return true;
}


void *term_input_handle( void )
{
    return GetStdHandle( STD_INPUT_HANDLE );
}

void *term_output_handle( void )
{
    return GetStdHandle( STD_OUTPUT_HANDLE );
}
