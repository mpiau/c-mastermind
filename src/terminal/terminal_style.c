#include "terminal/terminal_style.h"


struct Style s_currentStyle = STYLE_DEFAULT;


bool style_equals( struct Style const lhs, struct Style const rhs )
{
    return lhs.color == rhs.color && attr_equals( lhs.attr, rhs.attr );
}

struct Style style_current( void )
{
    return s_currentStyle;
}

void style_update( struct Style const style )
{
    s_currentStyle = style;
}

void style_reset( void )
{
    s_currentStyle = STYLE_DEFAULT;
}
