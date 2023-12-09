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


int term_print( enum TermColor color, char const *format, ... )
{
	va_list args;
	va_start ( args, format );
	printf( "%s", S_COLOR_STR[color] );
	vfprintf ( stdout, format, args );
	printf( "%s", S_COLOR_STR_RESET );
	va_end ( args );
}