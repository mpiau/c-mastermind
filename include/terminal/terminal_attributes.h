#pragma once

#include "core/core.h"

enum Attr
{
    Attr_NONE          = 0b00000000,
    Attr_BOLD          = 0b00000001,
    Attr_FAINT         = 0b00000010,
    Attr_ITALIC        = 0b00000100,
    Attr_UNDERLINE     = 0b00001000,
    Attr_BLINK         = 0b00010000,
    Attr_STRIKETHROUGH = 0b00100000,
    // One attribute available
    Attr_ALL           = 0b01111111,

    // The last bit is reserved for the implementation
    // To ensure its integrity, the code will assert if set by the user
};

typedef byte termattr;

bool attr_is_bold( termattr attr );
bool attr_is_faint( termattr attr );
bool attr_is_italic( termattr attr );
bool attr_is_underline( termattr attr );
bool attr_is_blink( termattr attr );
bool attr_is_strikethrough( termattr attr );

bool attr_equals( termattr lhs, termattr rhs );
