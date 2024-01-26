#include "ui.h"
#include "gameloop.h"
#include "keybindings.h"

#include <stdlib.h>


enum UIObjType
{
    UIObjType_BUTTON
};


struct UIObj
{
    u64 id;
    enum UIObjType type;
    union
    {
        struct UIButton button;
        struct UISimpleText simpleText;
    };
};

static u64 const S_INVALID_ID = (u64)-1;

static struct UIObj s_objs[128] = {};
static usize s_objCount = 0;
static u64 s_activeHoveredId = S_INVALID_ID;


static struct UIObj *register_object( enum UIObjType const type )
{
    if ( s_objCount == ARR_COUNT( s_objs ) ) return NULL;

    struct UIObj *obj = &s_objs[s_objCount];
    obj->id = s_objCount++;
    obj->type = type;

    return obj;
}


static inline struct UIButton *get_button_by_id( u64 const id )
{
    assert( id != S_INVALID_ID );
    if ( s_objs[id].type == UIObjType_BUTTON )
    {
        return &s_objs[id].button;
    }
    return NULL;
}


struct Style ui_style_button_text( u64 const id, struct UIButton const *button )
{
    if ( !button->enabled )
    {
        return STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
    }

    if ( s_activeHoveredId != id )
    {
        return STYLE( FGColor_WHITE );
    }

    return STYLE( FGColor_YELLOW );
}


struct Style ui_style_button_keybind( struct UIButton const *button )
{
    if ( !button->enabled )
    {
        return STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT | Attr_ITALIC );
    }

    return STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_ITALIC );
}


static void button_draw_internal( u64 const id )
{
    struct UIButton *button = get_button_by_id( id );
    screenpos const ul = rect_get_ul_corner( &button->rect );
    enum KeyInput const input = keybinding_get_binded_key( button->keybinding );

    button->shown = true;

	cursor_update_yx( ul.y, ul.x );
	style_update( ui_style_button_text( id, button ) );
	term_write( L"%S", button->text );

    if ( input == KeyInput_INVALID ) return;

    if ( button->rect.size.h > 1 )
    {
    	cursor_update_yx( ul.y + 1, ul.x );
    }

	style_update( ui_style_button_keybind( button ) );
	term_write( L"[%s]", key_input_get_name( input ) );
}



u64 uibutton_try_register( utf16 const *text, screenpos const pos, vec2u16 const size, enum KeyBinding keybinding, bool enabled )
{
    struct UIObj *obj = register_object( UIObjType_BUTTON );
    if ( !obj ) return S_INVALID_ID;

    assert( text );

    obj->button = (struct UIButton ) {
        .enabled = enabled,
        .shown = false,
        .text = text,
        .keybinding = keybinding,
        .rect = rect_make( pos, size )
    };
    return obj->id;
}


bool uibutton_clicked( u64 const id )
{
    if ( s_activeHoveredId != id ) return false;

    struct UIButton *button = get_button_by_id( id );
    if ( !button ) return false;

    enum KeyInput const input = keybinding_get_binded_key( button->keybinding );
    if ( input == KeyInput_INVALID ) return false;

    gameloop_emit_key( input );
    return true;
}


bool uibutton_is_enabled( u64 const id )
{
    return get_button_by_id( id )->enabled;
}


void uibutton_enable( u64 const id )
{
    get_button_by_id( id )->enabled = true;
}


void uibutton_disable( u64 const id )
{
    get_button_by_id( id )->enabled = false;
}


void uibutton_show( u64 const id )
{
    button_draw_internal( id );
}


bool uibutton_is_shown( u64 const id )
{
    return get_button_by_id( id )->shown;
}


void uibutton_hide( u64 const id )
{
    get_button_by_id( id )->shown = false;
    rect_clear( &get_button_by_id( id )->rect );
}


bool uibutton_is_hovered_by( u64 const id, screenpos const pos )
{
    return rect_is_inside( &get_button_by_id( id )->rect, pos );
}


bool uibutton_check_hovered( u64 id, screenpos pos )
{
    if ( !uibutton_is_enabled( id ) || !uibutton_is_shown( id ) || !uibutton_is_hovered_by( id, pos ) )
    {
        // If this button was the one hovered, then update the hover status and redraw it.
        if ( s_activeHoveredId == id )
        {
            s_activeHoveredId = S_INVALID_ID;            
            if ( uibutton_is_shown( id ) )
            {
                button_draw_internal( id );
            }
        }
        return false;
    }

    if ( s_activeHoveredId == id )
    {
        return true;
    }

    u64 const oldHovered = s_activeHoveredId;
    s_activeHoveredId = id;

    if ( oldHovered != S_INVALID_ID && uibutton_is_shown( oldHovered ) )
    {
        button_draw_internal( oldHovered ); // Remove the hovering color for the old button
    }

    button_draw_internal( id ); // Redraw with the updated status
    return true;
}


struct Rect const *uibutton_get_box( u64 const id )
{
    return &get_button_by_id( id )->rect;
}