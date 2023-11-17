#include "terminal.h"

#include <stdio.h>
#include <fcntl.h>
#include <windows.h>


static DWORD s_oldConsoleMode = 0;

static char const *term_get_clear_line( void )
{
	return "\x1b[2K";
}


static char const *term_get_move_beginning_prev_line( void )
{
	return "\x1b[1F";
}


void term_clear_last_line( void )
{
	printf( "%s%s", term_get_move_beginning_prev_line(), term_get_clear_line() );
}


static bool term_set_cursor_visibility( bool const visible )
{
	CONSOLE_CURSOR_INFO cursorInfo = {};

	HANDLE const hStdout = GetStdHandle( STD_OUTPUT_HANDLE );

	if ( !hStdout || !GetConsoleCursorInfo( hStdout, &cursorInfo ) )
	{
		return false;
	}

    cursorInfo.bVisible = visible;

    return SetConsoleCursorInfo( hStdout, &cursorInfo );
}


static void reset_console_mode_on_exit( void )
{
	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	SetConsoleMode( hStdin,  s_oldConsoleMode );

	term_set_cursor_visibility( true );
	printf( "%s", S_COLOR_STR_RESET );
}


static bool term_update_console_mode()
{
	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	if ( !hStdin ) { return false; }

	GetConsoleMode( hStdin, &s_oldConsoleMode );

	DWORD consoleModeIn  = s_oldConsoleMode & ~( ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT );
	if ( SetConsoleMode( hStdin,  consoleModeIn ) )
	{
		atexit( reset_console_mode_on_exit );
	}
}

bool term_init( void )
{
	// https://stackoverflow.com/questions/51726140/console-with-enable-line-input-doesnt-pass-r

	fflush( stdin ); // Important to flush before modifying the mode.
	_setmode( _fileno( stdin ), _O_BINARY );

	term_set_cursor_visibility( false );

	term_update_console_mode();
}


int term_print( enum TermColor color, char const *format, ... )
{
	va_list args;
	va_start ( args, format );
	printf( "%s", S_COLOR_STR[color] );
	vfprintf ( stdout, format, args );
	printf( "%s", S_COLOR_STR_RESET );
	va_end ( args );
}