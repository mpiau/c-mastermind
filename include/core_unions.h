#pragma once

#include "core_types.h"

typedef union vec2u16
{
    u16 raw[2];
    struct { u16 x, y; };
} vec2u16;

static_assert( sizeof( vec2u16 ) == 2 * sizeof( u16 ) );

typedef vec2u16 screenpos;


typedef union vec2u32
{
    u32 raw[2];
    struct { u32 x, y; };
} vec2u32;

static_assert( sizeof( vec2u32 ) == 2 * sizeof( u32 ) );
