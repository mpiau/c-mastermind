#include "terminal/terminal_attributes.h"

bool attr_is_bold( termattr const attr )
{
    return ( attr & Attr_BOLD ) != 0;
}

bool attr_is_faint( termattr const attr )
{
    return ( attr & Attr_FAINT ) != 0;
}

bool attr_is_italic( termattr const attr )
{
    return ( attr & Attr_ITALIC ) != 0;
}

bool attr_is_underline( termattr const attr )
{
    return ( attr & Attr_UNDERLINE ) != 0;
}

bool attr_is_blink( termattr const attr )
{
    return ( attr & Attr_BLINK ) != 0;
}

bool attr_is_strikethrough( termattr const attr )
{
    return ( attr & Attr_STRIKETHROUGH ) != 0;
}


bool attr_equals( termattr const lhs, termattr const rhs )
{
    return ( lhs & Attr_ALL ) == ( rhs & Attr_ALL );
}
