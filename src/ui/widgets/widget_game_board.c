#include "ui/widgets.h"
#include "rect.h"
#include "ui.h"
#include "events.h"
#include "requests.h"
#include "keybindings.h"

#include <stdlib.h>


enum // Constants
{
    TOTAL_BOARD_WIDTH = 78
};

enum ButtonIdx
{
	ButtonIdx_CONFIRM_TURN,
	ButtonIdx_RESET_TURN,
	ButtonIdx_ABANDON_GAME,

	ButtonIdx_Count
};


struct WidgetGameBoard
{
    struct Widget base;

	struct Rect box;
    u64 buttons[ButtonIdx_Count];
};


static inline void draw_character_n_times( utf16 const character, usize const n )
{
	for ( usize x = 0; x < n; ++x )
	{
		term_write( L"%lc", character );
	}
}


static void draw_internal_board_lines( struct WidgetGameBoard *widget )
{
	screenpos const ul = rect_get_ul_corner( &widget->box );

	// Lines separating the turns
	style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
	for( usize idx = 0; idx < 3; idx++ )
	{
		cursor_update_yx( ul.y + 6 * ( idx + 1 ), ul.x + 2 ); 
		draw_character_n_times( L'─', TOTAL_BOARD_WIDTH - 4 );
	}

	// Line that separates buttons from the rest of the game
	cursor_update_yx( ul.y + widget->box.size.h - 3, ul.x );
	style_update( STYLE( FGColor_BRIGHT_BLACK ) );
	term_write( L"%lc", L'├' );
	draw_character_n_times( L'─', TOTAL_BOARD_WIDTH - 2 );
	term_write( L"%lc", L'┤' );
}


static void on_trigger_confirm_turn( bool )
{
    struct Request const req = (struct Request) {
        .type = RequestType_CONFIRM_TURN
    };
    request_send( &req );
}


static void on_trigger_reset_turn( bool )
{
    struct Request const req = (struct Request) {
        .type = RequestType_RESET_TURN
    };
    request_send( &req );
}


static void on_trigger_abandon_game( bool )
{
    struct Request const req = (struct Request) {
        .type = RequestType_ABANDON_GAME
    };
    request_send( &req );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetGameBoard *widget = (struct WidgetGameBoard *)subscriber;

    switch( event->type )
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

            if ( event->userInput.input == keybinding_get_binded_key( KeyBinding_NEXT ) )
            {
                struct Request req = (struct Request) {
                    .type = RequestType_NEXT
                };
                request_send( &req );
            }
            else if ( event->userInput.input == keybinding_get_binded_key( KeyBinding_PREVIOUS ) )
            {
                struct Request req = (struct Request) {
                    .type = RequestType_PREVIOUS
                };
                request_send( &req );
            }

            break;
        }
    }
    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
	struct WidgetGameBoard *widget = (struct WidgetGameBoard *)base;
	rect_draw_borders( &widget->box, L"Mastermind Board" );
	draw_internal_board_lines( widget );

	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_show( widget->buttons[idx] );
	}    
}


static void disable_callback( struct Widget *base )
{
    struct WidgetGameBoard *widget = (struct WidgetGameBoard *)base;
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_hide( widget->buttons[idx] );
	}
	rect_clear( &widget->box );    
}


struct Widget *widget_game_board_create( void )
{
    struct WidgetGameBoard *const widget = calloc( 1, sizeof( struct WidgetGameBoard ) );
    if ( !widget ) return NULL;

    widget->base.name = "GameBoard";
    widget->base.enabledScenes = UIScene_IN_GAME;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    // Widget specific

  	widget->box = rect_make( SCREENPOS( 17, 2 ), VEC2U16( TOTAL_BOARD_WIDTH, 27 ) );

	screenpos const bul = SCREENPOS( 18, 27 );
    widget->buttons[ButtonIdx_CONFIRM_TURN] = uibutton_register( L"Confirm Turn", SCREENPOS( bul.x + 3, bul.y ), VEC2U16( 19, 1 ), Keybinding_CONFIRM_TURN, on_trigger_confirm_turn, true );
    widget->buttons[ButtonIdx_RESET_TURN]   = uibutton_register( L"Reset Turn", SCREENPOS( bul.x + 25, bul.y ), VEC2U16( 13, 1 ), Keybinding_RESET_TURN, on_trigger_reset_turn, true );
    widget->buttons[ButtonIdx_ABANDON_GAME] = uibutton_register( L"Abandon Game", SCREENPOS( bul.x + 58, bul.y ), VEC2U16( 15, 1 ), Keybinding_ABANDON_GAME, on_trigger_abandon_game, true );

    event_register( widget, on_event_callback );
    event_subscribe( widget, EventType_MaskAll );

    return (struct Widget *)widget;
}