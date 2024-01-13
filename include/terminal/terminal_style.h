#pragma once

#include "core_types.h"

#include "terminal/terminal_attributes.h"
#include "terminal/terminal_colors.h"

struct Style
{
    termattr  attr;
    termcolor color;
};
static_assert( sizeof( struct Style ) == 2 );

#define STYLE_WITH_ATTR( _color, _attr ) ( (struct Style) { .attr = _attr, .color = _color } )
#define STYLE( _color )                  STYLE_WITH_ATTR( _color, Attr_NONE )
#define STYLE_DEFAULT                    STYLE( Color_DEFAULT )

bool style_equals( struct Style lhs, struct Style rhs );

struct Style style_current( void );
void style_update( struct Style style );
void style_reset( void );
