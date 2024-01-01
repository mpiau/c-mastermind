#pragma once

#include "core_types.h"
#include "core_unions.h"

typedef void ( *OnResizeCallback )( vec2u16 oldSize, vec2u16 newSize );

bool console_screen_init( void const *handle ); /* HANDLE */

void console_screen_frame( void );
bool console_screen_resize( vec2u16 newSize );
//bool console_screen_is_being_resized( void );

bool console_screen_is_too_small( void );
bool console_screen_is_width_too_small( void );
bool console_screen_is_height_too_small( void );

bool console_screen_register_on_resize_callback( OnResizeCallback callback );
vec2u16 console_screen_get_size( void );
