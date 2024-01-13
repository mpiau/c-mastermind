#include "terminal/terminal_cursor.h"

#include <stdio.h>

static screenpos s_currentPos = SCREENPOS( 1, 1 );

screenpos cursor_pos( void )
{
    return s_currentPos;
}


void cursor_update_y( u16 const y )
{
    s_currentPos.y = y;
}

void cursor_update_x( u16 const x )
{
    s_currentPos.x = x;
}

void cursor_update_yx( u16 const y, u16 const x )
{
    s_currentPos = SCREENPOS( x, y );
}

void cursor_update_pos( screenpos const pos )
{
    s_currentPos = pos;
}


void cursor_move_right_by( u16 const n )
{
    s_currentPos.x += n;
}

void cursor_move_left_by( u16 const n )
{
    if ( s_currentPos.x < n )
        s_currentPos.x = 0;
    else
        s_currentPos.x -= n;
}

void cursor_move_top_by( u16 const n )
{
    if ( s_currentPos.y < n )
        s_currentPos.y = 0;
    else
        s_currentPos.y -= n;
}

void cursor_move_bottom_by( u16 const n )
{
    s_currentPos.y += n;
}


void cursor_hide( void )
{
    wprintf( L"\x1B[?25l" );
}

void cursor_show( void )
{
    wprintf( L"\x1B[?25h" );
}

void cursor_start_blinking( void )
{
    wprintf( L"\x1B[?12h" );
}

void cursor_stop_blinking( void )
{
    wprintf( L"\x1B[?12l" );
}
