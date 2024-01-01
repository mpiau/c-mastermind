#pragma once

#include "core_types.h"

typedef union vec2u16
{
    u16 raw[2];
    struct { u16 x, y; };
	struct { u16 w, h; };
} vec2u16;

static_assert( sizeof( vec2u16 ) == 2 * sizeof( u16 ) );

typedef vec2u16 screenpos;

typedef union vec2u32
{
    u32 raw[2];
    struct { u32 x, y; };
	struct { u32 w, h; };
} vec2u32;

static_assert( sizeof( vec2u32 ) == 2 * sizeof( u32 ) );


#define VEC2U16( _x, _y )   ( (vec2u16) { .x = _x, .y = _y } )
#define VEC2U32( _x, _y )   ( (vec2u32) { .x = _x, .y = _y } )
#define SCREENPOS( _x, _y ) ( (screenpos) { .x = _x, .y = _y } )
