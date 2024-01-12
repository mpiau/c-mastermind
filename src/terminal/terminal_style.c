#include "terminal/terminal_style.h"

static struct TermStyle s_currentStyle = {};


struct TermStyle term_style_make( termcolor const color, enum Properties const formatting )
{
    return (struct TermStyle) {
        .color = color,
        .properties = formatting
    };
}


struct TermStyle term_style_default( void )
{
    return term_style_make( COLOR_DEFAULT, Properties_NONE );
}


struct TermStyle term_style_current( void )
{
    return s_currentStyle;
}


void term_style_set_current( struct TermStyle const style )
{
    s_currentStyle = style;
}


bool term_style_equals( struct TermStyle const lhs, struct TermStyle const rhs )
{
    // As the last bit of the properties is reserved for something else
    // We want to exclude it first before doing any properties comparison.
    return lhs.color == rhs.color && ( lhs.properties & Properties_ALL ) == ( rhs.properties & Properties_ALL );
}


bool term_style_is_bold( byte const properties )
{
    return ( properties & Properties_BOLD ) != 0;
}


bool term_style_is_faint( byte const properties )
{
    return ( properties & Properties_FAINT ) != 0;
}


bool term_style_is_italic( byte const properties )
{
    return ( properties & Properties_ITALIC ) != 0;
}


bool term_style_is_underline( byte const properties )
{
    return ( properties & Properties_UNDERLINE ) != 0;
}
