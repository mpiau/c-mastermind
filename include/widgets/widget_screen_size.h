#pragma once

#include "core_types.h"
#include "core_unions.h"

// Forward declaration
struct Widget;

struct Widget *widget_screen_size_create( void );

void widget_screen_size_size_update( struct Widget *widget, vec2u16 newSize );
