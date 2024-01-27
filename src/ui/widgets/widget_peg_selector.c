#include "ui/widgets.h"

#include "terminal/terminal.h"
#include "rect.h"
#include "settings.h"
#include "ui.h"
#include "keybindings.h"
#include "events.h"
#include "requests.h"
#include "mastermind.h"

#include <stdlib.h>


enum ButtonIdx
{
    ButtonIdx_PEG_BLACK,
    ButtonIdx_PEG_RED,
    ButtonIdx_PEG_GREEN,
    ButtonIdx_PEG_YELLOW,
    ButtonIdx_PEG_BLUE,
    ButtonIdx_PEG_MAGENTA,
    ButtonIdx_PEG_CYAN,
    ButtonIdx_PEG_WHITE,

    ButtonIdx_Count
};


struct WidgetPegSelector
{
    struct Widget base;

    struct Rect box;
    u64 buttons[ButtonIdx_Count];
};


static void create_peg_request( gamepiece const piece, bool const clicked )
{
    struct Request req;
    if ( clicked )
    {
        req = (struct Request) {
            .type = RequestType_PEG_SELECT,
            .pegSelect = (struct RequestPegSelect) {
                .piece = piece
            }
        };
    }
    else
    {
        req = (struct Request) {
            .type = RequestType_PEG_ADD,
            .pegAdd = (struct RequestPegAdd) {
                .piece = piece
            }
        };
    }

    request_send( &req );
}


static void on_trigger_black_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_BLACK, clicked );
}


static void on_trigger_red_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_RED, clicked );
}


static void on_trigger_green_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_GREEN, clicked );
}


static void on_trigger_yellow_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_YELLOW, clicked );
}


static void on_trigger_cyan_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_CYAN, clicked );
}


static void on_trigger_magenta_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_MAGENTA, clicked );
}


static void on_trigger_blue_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_BLUE, clicked );
}


static void on_trigger_white_peg( bool const clicked )
{
    create_peg_request( Piece_PEG_WHITE, clicked );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetPegSelector *widget = (struct WidgetPegSelector *)subscriber;

    switch ( event->type )
    {
        case EventType_MOUSE_MOVED:
        {
            for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
            {
                if ( uibutton_check_hovered( widget->buttons[idx], event->mouseMoved.pos ) )
                {
                    break;
                }
            }
            break;
        }
        case EventType_USER_INPUT:
        {
            for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
            {
                if ( uibutton_check_interaction( widget->buttons[idx], event->userInput.input ) )
                {
                    return EventPropagation_STOP;
                }
            }
            break;
        }
        case EventType_PEG_ADDED:
        {
            if ( !settings_is_duplicate_allowed() )
            {
                enum Piece const idx = event->pegAdded.piece & Piece_MaskColor;
                uibutton_desactivate( widget->buttons[idx] );
            }
            break;
        }
        case EventType_PEG_REMOVED:
        {
            if ( !settings_is_duplicate_allowed() )
            {
                enum Piece const idx = event->pegRemoved.piece & Piece_MaskColor;
                uibutton_activate( widget->buttons[idx] );
            }
            break;
        }
    }

    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
    struct WidgetPegSelector *widget = (struct WidgetPegSelector *)base;
    rect_draw_borders( &widget->box, L"Pegs" );

   	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_show( widget->buttons[idx] );
	    screenpos ul = rect_get_ul_corner( uibutton_get_box( widget->buttons[idx] ) );
	    ul.x += 6;
    	piece_write_4x2( ul, (enum Piece)idx );
	}    
}


static void disable_callback( struct Widget *base )
{
    struct WidgetPegSelector *widget = (struct WidgetPegSelector *)base;

   	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_hide( widget->buttons[idx] );
	}
    rect_clear_borders( &widget->box );
}


struct Widget *widget_peg_selector_create( void )
{
    struct WidgetPegSelector *const widget = calloc( 1, sizeof( struct WidgetPegSelector ) );
    if ( !widget ) return NULL;

    widget->base.name = "PegSelector";
    widget->base.enabledScenes = UIScene_IN_GAME;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    /* Widget specific */

    screenpos ul = SCREENPOS( 3, 2 );
    vec2u16 const size = VEC2U16( 10, 2 );
	widget->box = rect_make( ul, VEC2U16( 14, 27 ) );
    ul.x += 2;
    widget->buttons[ButtonIdx_PEG_BLACK]   = uibutton_register( L"Grey", SCREENPOS( ul.x, ul.y + 2 ), size, KeyBinding_PEG_BLACK, on_trigger_black_peg, true );
    widget->buttons[ButtonIdx_PEG_RED]     = uibutton_register( L"Red", SCREENPOS( ul.x, ul.y + 5 ), size, KeyBinding_PEG_RED, on_trigger_red_peg, true );
    widget->buttons[ButtonIdx_PEG_GREEN]   = uibutton_register( L"Green", SCREENPOS( ul.x, ul.y + 8 ), size, KeyBinding_PEG_GREEN, on_trigger_green_peg, true );
    widget->buttons[ButtonIdx_PEG_YELLOW]  = uibutton_register( L"Yell.", SCREENPOS( ul.x, ul.y + 11 ), size, KeyBinding_PEG_YELLOW, on_trigger_yellow_peg, true );
    widget->buttons[ButtonIdx_PEG_BLUE]    = uibutton_register( L"Blue", SCREENPOS( ul.x, ul.y + 14 ), size, KeyBinding_PEG_BLUE, on_trigger_blue_peg, true );
    widget->buttons[ButtonIdx_PEG_MAGENTA] = uibutton_register( L"Purp.", SCREENPOS( ul.x, ul.y + 17 ), size, KeyBinding_PEG_MAGENTA, on_trigger_magenta_peg, true );
    widget->buttons[ButtonIdx_PEG_CYAN]    = uibutton_register( L"Cyan", SCREENPOS( ul.x, ul.y + 20 ), size, KeyBinding_PEG_CYAN, on_trigger_cyan_peg, true );
    widget->buttons[ButtonIdx_PEG_WHITE]   = uibutton_register( L"White", SCREENPOS( ul.x, ul.y + 23 ), size, KeyBinding_PEG_WHITE, on_trigger_white_peg, true );

    event_register( widget, on_event_callback );
    event_subscribe( widget, EventType_MaskAll );

    return (struct Widget *)widget;
}
