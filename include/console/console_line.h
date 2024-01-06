#pragma once

#include "core_types.h"
#include "core_unions.h"

// An horizontal element that has a height of one in the console.

struct ConLine
{
    screenpos startPos;
    screenpos endPos;
    u16 width;
};

struct ConLine console_line_make( screenpos start, usize width );
