#include "button.h"

#include "terminal/terminal.h"

static struct Style get_text_style( bool const enabled, bool const hovered )
{
    if ( !enabled )
    {
        return STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
    }

    if ( !hovered )
    {
        return STYLE( FGColor_WHITE );
    }

    return STYLE( FGColor_YELLOW );
}

static struct Style get_key_style( bool const enabled )
{
    if ( !enabled )
    {
        return STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT | Attr_ITALIC );
    }

    return STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_ITALIC );
}


struct Button button_make( utf16 const *name, screenpos const ul, vec2u16 const size, enum KeyInput const bindedKey, bool enabled )
{
    return (struct Button) {
        .enabled = enabled,
        .rect = rect_make( ul, size ),
        .bindedKey = bindedKey,
        .name = name
    };
}


void button_write( struct Button const *button, bool const isHovered )
{
    struct Style const textStyle = get_text_style( button->enabled, isHovered );
    struct Style const keyStyle = get_key_style( button->enabled );

    screenpos const ul = rect_get_ul_corner( &button->rect );

	cursor_update_yx( ul.y, ul.x );
	style_update( textStyle );
	term_write( L"%S", button->name );

    if ( button->rect.size.h > 1 )
    {
    	cursor_update_yx( ul.y + 1, ul.x );
    }

	style_update( keyStyle );
	term_write( L"[%s]", key_input_get_name( button->bindedKey ) );
}


bool button_is_hovered_by( struct Button const *button, screenpos pos )
{
    return rect_is_inside( &button->rect, pos );
}


bool button_is_enabled( struct Button const *button )
{
    return button->enabled;
}


void button_enable( struct Button *button )
{
    button->enabled = true;
}


void button_disable( struct Button *button )
{
    button->enabled = false;
}
