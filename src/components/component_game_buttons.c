#include "components/component_game_buttons.h"

#include "components/component_header.h"
#include "keyboard_inputs.h"
#include "rect.h"
#include "ui.h"

#include <stdlib.h>


enum ButtonIdx
{
	ButtonIdx_NEW_GAME,
	ButtonIdx_GAME_RULES,
	ButtonIdx_SETTINGS,
	ButtonIdx_QUIT,

	ButtonIdx_Count
};

struct ComponentGameButtons
{
	struct ComponentHeader header;
	u64 buttons[ButtonIdx_Count];
};
#define CAST_TO_COMP( _header ) ( ( struct ComponentGameButtons * )( _header ) )


static void on_mouse_move_callback( struct ComponentHeader *header, screenpos const pos )
{
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		if ( uibutton_check_hovered( CAST_TO_COMP( header )->buttons[idx], pos ) )
		{
			return;
		}
	}
}


static bool on_input_received_callback( struct ComponentHeader *header, enum KeyInput input )
{
	if ( input != KeyInput_MOUSE_BTN_LEFT ) return false;

	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		if ( uibutton_check_interaction( CAST_TO_COMP( header )->buttons[idx], input ) )
		{
			return true;
		}
	}

	return false;
}


static void enable_callback( struct ComponentHeader *header )
{
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_show( CAST_TO_COMP( header )->buttons[idx] );
	}
}


static void disable_callback( struct ComponentHeader *header )
{
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_hide( CAST_TO_COMP( header )->buttons[idx] );
	}
}


struct ComponentHeader *component_game_buttons_create( void )
{
    struct ComponentGameButtons *const comp = calloc( 1, sizeof( struct ComponentGameButtons ) );
    if ( !comp ) return NULL;

	comp->header.id = ComponentId_GAME_BUTTONS;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
	callbacks->enableCb = enable_callback;
	callbacks->disableCb = disable_callback;
	callbacks->mouseMoveCb = on_mouse_move_callback;
	callbacks->inputReceivedCb = on_input_received_callback;

	// Specific data
	u64 *ids = comp->buttons;
	ids[ButtonIdx_NEW_GAME]   = uibutton_register( L"New Game", SCREENPOS( 84, 29 ), VEC2U16( 8, 2 ), Keybinding_NEW_GAME, NULL, true );
	ids[ButtonIdx_GAME_RULES] = uibutton_register( L"Game Rules", SCREENPOS( 94, 29 ), VEC2U16( 10, 2 ), KeyBinding_OPEN_GAME_RULES, NULL, false );
	ids[ButtonIdx_SETTINGS]   = uibutton_register( L"Settings", SCREENPOS( 106, 29 ), VEC2U16( 8, 2 ), KeyBinding_OPEN_SETTINGS_MENU, NULL, false );
	ids[ButtonIdx_QUIT]       = uibutton_register( L"Quit", SCREENPOS( 116, 29 ), VEC2U16( 4, 2 ), KeyBinding_QUIT, NULL, true );

	return (struct ComponentHeader *)comp;
}
