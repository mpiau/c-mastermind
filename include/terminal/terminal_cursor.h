#pragma once

#include "core/core.h"

screenpos cursor_pos( void );

void cursor_update_y( u16 y );
void cursor_update_x( u16 x );
void cursor_update_yx( u16 y, u16 x );
void cursor_update_pos( screenpos pos );

void cursor_move_right_by( u16 n );
void cursor_move_left_by( u16 n );
void cursor_move_top_by( u16 n );
void cursor_move_bottom_by( u16 n );

void cursor_hide( void );
void cursor_show( void );
void cursor_start_blinking( void );
void cursor_stop_blinking( void );
