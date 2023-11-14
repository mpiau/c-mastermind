#include "terminal.h"

#include <stdio.h>

/*
printf("\033[XA"); // Move up X lines;
printf("\033[XB"); // Move down X lines;
printf("\033[XC"); // Move right X column;
printf("\033[XD"); // Move left X column;
printf("\033[2J"); // Clear screen
*/

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