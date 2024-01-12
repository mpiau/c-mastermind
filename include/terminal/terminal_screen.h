#pragma once

#include "core_types.h"
#include "core_unions.h"

enum // Constants
{
    // Maximum supported size of the generated content in a single call of term_screen_write() / term_screen_refresh().
    TERM_SCREEN_WRITE_MAXIMUM_BUFFER = 256,
    TERM_SCREEN_REFRESH_MAXIMUM_BUFFER = 8096
};

bool term_screen_init( void const *handle );

// Write/clear won't have any impact on the content displayed until refresh is called.
int term_screen_write( utf16 const *format, ... );
void term_screen_clear( void );
void term_screen_refresh( void );

void term_screen_on_resize( screensize newSize );

// Getters
screensize term_screen_current_size( void );
screenpos term_screen_cursor_pos( void );

// Setters
void term_screen_set_cursor_pos( screenpos pos );
