#include "ui/widgets.h"

#include "keyboard_inputs.h"
#include "ui.h"
#include "events.h"
#include "requests.h"

#include <stdlib.h>

enum ButtonIdx
{
	ButtonIdx_NEW_GAME,
	ButtonIdx_GAME_RULES,
	ButtonIdx_SETTINGS,
	ButtonIdx_QUIT,

	ButtonIdx_Count
};

struct WidgetBottomNav
{
    struct Widget base;
    u64 buttons[ButtonIdx_Count];
};


static void on_trigger_new_game( bool )
{
    struct Request request = (struct Request) {
        .type = RequestType_START_NEW_GAME
    };
    request_send( &request );
}


static void on_trigger_game_rules( bool )
{
    // Do nothing for now
}


static void on_trigger_settings( bool )
{
    // Do nothing for now
}


static void on_trigger_quit( bool )
{
    struct Request request = (struct Request) {
        .type = RequestType_EXIT_APP
    };
    request_send( &request );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetBottomNav *widget = ( struct WidgetBottomNav * )subscriber;
    if ( event->type == EventType_MOUSE_MOVED )
    {
        for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
        {
            if ( uibutton_check_hovered( widget->buttons[idx], event->mouseMoved.pos ) )
            {
                break;
            }
        }
    }
    else if ( event->type == EventType_USER_INPUT )
    {
        for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
        {
            if ( uibutton_check_interaction( widget->buttons[idx], event->userInput.input ) )
            {
                return EventPropagation_STOP;
            }
        }
    }

    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *widget )
{
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_show( ( (struct WidgetBottomNav *)widget )->buttons[idx] );
	}
}


static void disable_callback( struct Widget *widget )
{
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_hide( ( (struct WidgetBottomNav *)widget )->buttons[idx] );
	}
}


struct Widget *widget_bottom_nav_create( void )
{
    struct WidgetBottomNav *const widget = calloc( 1, sizeof( struct WidgetBottomNav ) );
    if ( !widget ) return NULL;

    widget->base.name = "BottomNavigation";
    widget->base.enabledScenes = UIScene_ALL;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

	u64 *ids = widget->buttons;
	ids[ButtonIdx_NEW_GAME]   = uibutton_register( L"New Game", SCREENPOS( 84, 29 ), VEC2U16( 8, 2 ), Keybinding_NEW_GAME, on_trigger_new_game, true );
	ids[ButtonIdx_GAME_RULES] = uibutton_register( L"Game Rules", SCREENPOS( 94, 29 ), VEC2U16( 10, 2 ), KeyBinding_OPEN_GAME_RULES, on_trigger_game_rules, false );
	ids[ButtonIdx_SETTINGS]   = uibutton_register( L"Settings", SCREENPOS( 106, 29 ), VEC2U16( 8, 2 ), KeyBinding_OPEN_SETTINGS_MENU, on_trigger_settings, false );
	ids[ButtonIdx_QUIT]       = uibutton_register( L"Quit", SCREENPOS( 116, 29 ), VEC2U16( 4, 2 ), KeyBinding_QUIT, on_trigger_quit, true );

    event_register( widget, on_event_callback );
    event_subscribe( widget, EventType_MOUSE_MOVED | EventType_USER_INPUT );

    return (struct Widget *)widget;
}
