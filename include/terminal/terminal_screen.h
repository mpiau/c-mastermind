#pragma once

#include "core_types.h"
#include "core_unions.h"

enum // Constants
{
    // Maximum supported size of the generated content in a single call of term_write() / term_refresh().
    TERM_WRITE_BUFFER_SIZE = 256,
    TERM_REFRESH_BUFFER_SIZE = 16192
};

bool term_screen_init( void const *handle );

// Write/clear won't have any impact on the content displayed until refresh is called.
int term_write( utf16 const *format, ... );
void term_clear( void );
void term_refresh( void );

void term_on_resize( screensize newSize );



// Getters
screensize term_size( void );

struct Character term_character_buffered_at_pos( screenpos pos );
