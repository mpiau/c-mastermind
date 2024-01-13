#pragma once

#include "terminal/terminal_cursor.h"
#include "terminal/terminal_screen.h"
#include "terminal/terminal_style.h"


bool term_init( char const *optTitle, bool onDedicatedConsole );
void term_uninit( void );
bool term_is_init( void );

bool term_set_title( char const *title );

// HANDLE
void *term_input_handle( void );
void *term_output_handle( void );

