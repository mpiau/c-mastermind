#pragma once

#include "core_types.h"

typedef union vec2u16
{
    u16 raw[2];
    struct { u16 x, y; };
} vec2u16;

static_assert( sizeof( vec2u16 ) == 2 * sizeof( u16 ) );
