#pragma once

#include "core_types.h"

bool console_global_init( char const *optTitle );
void console_global_uninit( void );
bool console_is_global_init( void );

void console_print( char const *buffer );

bool console_cursor_show( void );
bool console_cursor_hide( void );
bool console_cursor_is_shown( void );

// bool console_screen_get_size( void );
// bool console_screen_has_been_resized( void );
// bool console_screen_is_too_small( void );
