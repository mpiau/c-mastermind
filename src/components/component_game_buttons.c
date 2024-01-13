#include "components/component_game_buttons.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "game.h"
#include "keyboard_inputs.h"
#include "gameloop.h"
#include "rect.h"
#include "mouse.h"
#include "mastermind.h"

#include <stdlib.h>
#include <string.h>


enum ButtonId
{
	// Upper row
	ButtonId_NEW_GAME,
	ButtonId_ABANDON_GAME,
	ButtonId_GAME_RULES,
	ButtonId_SETTINGS,
	ButtonId_QUIT,

	// Bottom row
	ButtonId_PREVIOUS,
	ButtonId_NEXT,
	ButtonId_BOARD,
	ButtonId_PEGS,
	ButtonId_PLACE_SELECT,
	ButtonId_ERASE_UNSELECT,
	ButtonId_VALIDATE,
	ButtonId_RESET,
	ButtonId_HISTORY_UP,
	ButtonId_HISTORY_DOWN,
	ButtonId_GameButtonsBegin = ButtonId_PREVIOUS,
	ButtonId_GameButtonsEnd = ButtonId_HISTORY_DOWN,

	ButtonId_Count,

	// Special edge case, impl only.
	ButtonId_Invalid = ButtonId_Count
};

enum ButtonStatus
{
	ButtonStatus_ENABLED,	// The user can interact with it, and colors and shown
	ButtonStatus_DISABLED,  // The user can't interact with it, and displayed greyed out.
	ButtonStatus_HIDDEN	    // The button isn't displayed on the screen at all.
};

struct Button
{
	struct Rect box;
	utf16 const *text;
	enum ButtonStatus status;
	enum KeyInput bindedKey;
};

struct ComponentGameButtons
{
	struct ComponentHeader header;

	struct Button buttons[ButtonId_Count];
	enum ButtonId hoveredButton;
};
#define CAST_TO_COMPONENT( _header ) ( ( struct ComponentGameButtons * )( _header ) )


static void button_get_hovered_style( enum ButtonId const id, struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	switch( id )
	{
		case ButtonId_VALIDATE:
			*outTextStyle = STYLE( FGColor_GREEN );
			*outKeyStyle  = STYLE( FGColor_BRIGHT_GREEN );
			break;

		case ButtonId_ABANDON_GAME:
		case ButtonId_RESET:
			*outTextStyle = STYLE( FGColor_RED );
			*outKeyStyle  = STYLE( FGColor_BRIGHT_RED );
			break;

		default:
			*outTextStyle = STYLE( FGColor_YELLOW );
			*outKeyStyle  = STYLE( FGColor_BRIGHT_YELLOW );
			break;
	}
}

static void button_get_default_style( enum ButtonId const id, struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	switch( id )
	{
		case ButtonId_VALIDATE:
			*outKeyStyle = STYLE( FGColor_GREEN );
			break;

		case ButtonId_ABANDON_GAME:
		case ButtonId_RESET:
			*outKeyStyle = STYLE( FGColor_RED );
			break;

		default:
			*outKeyStyle = STYLE( FGColor_YELLOW );
			break;
	}

	*outTextStyle = STYLE( FGColor_BRIGHT_BLACK );
}

static void button_get_disabled_style( struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	*outTextStyle = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
	*outKeyStyle  = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
}


static void game_buttons_update_status( struct ComponentHeader *header, enum ButtonStatus const status )
{
	struct ComponentGameButtons *widget = (struct ComponentGameButtons *)header;
	screenpos const mousePosition = mouse_pos();

	for ( enum ButtonId idx = ButtonId_GameButtonsBegin; idx <= ButtonId_GameButtonsEnd; ++idx )
	{
		struct Button *button = &widget->buttons[idx];
		button->status = status;
		if ( status == ButtonStatus_ENABLED && rect_is_inside( &button->box, mousePosition ) )
		{
			widget->hoveredButton = idx;
		}
	}
}


static void on_mouse_move_callback( struct ComponentHeader *const widget, screenpos const pos )
{
	struct ComponentGameButtons *boardButtons = (struct ComponentGameButtons *)widget;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		if ( boardButtons->buttons[idx].status == ButtonStatus_ENABLED && rect_is_inside( &boardButtons->buttons[idx].box, pos ) )
		{
			if ( boardButtons->hoveredButton != idx )
			{
				boardButtons->hoveredButton = idx;
				widget->refreshNeeded = true;
			}
			return;
		}
	}

	if ( boardButtons->hoveredButton != ButtonId_Invalid )
	{
		boardButtons->hoveredButton = ButtonId_Invalid;
		widget->refreshNeeded = true;
	}
}


static void on_game_update_callback( struct ComponentHeader *header, enum GameUpdateType type )
{
	struct ComponentGameButtons *boardButtons = (struct ComponentGameButtons *)header;

	if ( type == GameUpdateType_GAME_FINISHED )
	{
		struct Button *abandonButton = &boardButtons->buttons[ButtonId_ABANDON_GAME];
		abandonButton->status = ButtonStatus_DISABLED;
		if ( boardButtons->hoveredButton == ButtonId_ABANDON_GAME )
		{
			boardButtons->hoveredButton = ButtonId_Invalid;
		}
		game_buttons_update_status( header, ButtonStatus_HIDDEN );
		header->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_GAME_NEW )
	{
		struct Button *abandonButton = &boardButtons->buttons[ButtonId_ABANDON_GAME];
		abandonButton->status = ButtonStatus_ENABLED;
		if ( rect_is_inside( &abandonButton->box, mouse_pos() ) )
		{
			boardButtons->hoveredButton = ButtonId_ABANDON_GAME;
		}
		game_buttons_update_status( header, ButtonStatus_ENABLED );
		header->refreshNeeded = true;
	}
}


static bool on_input_received_callback( struct ComponentHeader *header, enum KeyInput input )
{
	struct ComponentGameButtons *buttons = CAST_TO_COMPONENT( header );
	if ( input == KeyInput_MOUSE_BTN_LEFT && buttons->hoveredButton != ButtonId_Invalid )
	{
		gameloop_emit_key( buttons->buttons[buttons->hoveredButton].bindedKey );
		return true;
	}

	return false;
}


static void on_refresh_callback( struct ComponentHeader const *widget )
{
	struct ComponentGameButtons const *boardButtons = (struct ComponentGameButtons const *)widget;

	struct Style textStyle;
	struct Style keyStyle;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		struct Button const *button = &boardButtons->buttons[idx];

		screenpos const ul = rect_get_ul_corner( &button->box );
		bool const isHovered = ( boardButtons->hoveredButton == idx );

		cursor_update_pos( ul );
		if ( button->status == ButtonStatus_ENABLED )
		{
			isHovered ? button_get_hovered_style( idx, &textStyle, &keyStyle ) : button_get_default_style( idx, &textStyle, &keyStyle );

			style_update( textStyle );
			term_write( L"[" );

			style_update( keyStyle );
			term_write( L"%lc", button->text[0] );

			style_update( textStyle );
			term_write( L"%S]", &button->text[1] );
		}
		else if ( button->status == ButtonStatus_DISABLED )
		{
			button_get_disabled_style( &textStyle, &keyStyle );
			style_update( textStyle );
			term_write( L"[%S]", button->text );
		}
		else
		{
			term_write( L"%*lc", wcslen( button->text ) + 2, L' ' ); // + 2 for '[' and ']'
		}
	}
}


static inline struct Button button_make( screenpos const ul, vec2u16 const size, utf16 const *const text, enum ButtonStatus const status, enum KeyInput const bindedKey )
{
	assert( text != NULL );

	return (struct Button) {
		.box = rect_make( ul, size ),
		.text = text,
		.status = status,
		.bindedKey = bindedKey
	};
}


static void init_component_data( struct ComponentGameButtons *comp )
{
	comp->hoveredButton = ButtonId_Invalid;

	struct Button *buttons = comp->buttons;
	// Upper row
	buttons[ButtonId_NEW_GAME]     = button_make( SCREENPOS( 64, 1 ), VEC2U16( 10, 1 ), L"New Game", ButtonStatus_ENABLED, KeyInput_N );
	buttons[ButtonId_ABANDON_GAME] = button_make( SCREENPOS( 75, 1 ), VEC2U16( 14, 1 ), L"Abandon Game", ButtonStatus_DISABLED, KeyInput_A );
	buttons[ButtonId_GAME_RULES]   = button_make( SCREENPOS( 90, 1 ), VEC2U16( 12, 1 ), L"Game Rules", ButtonStatus_DISABLED, KeyInput_G );
	buttons[ButtonId_SETTINGS]     = button_make( SCREENPOS( 103, 1 ), VEC2U16( 10, 1 ), L"Settings", ButtonStatus_DISABLED, KeyInput_S );
	buttons[ButtonId_QUIT]         = button_make( SCREENPOS( 114, 1 ), VEC2U16( 6, 1 ), L"Quit", ButtonStatus_ENABLED, KeyInput_Q );
	// Bottom row
	buttons[ButtonId_PREVIOUS]       = button_make( SCREENPOS( 1, 30 ), VEC2U16( 7, 1 ), L"←Prev", ButtonStatus_HIDDEN, KeyInput_ARROW_LEFT );
	buttons[ButtonId_NEXT]           = button_make( SCREENPOS( 8, 30 ), VEC2U16( 7, 1 ), L"→Next", ButtonStatus_HIDDEN, KeyInput_ARROW_RIGHT );
	buttons[ButtonId_BOARD]          = button_make( SCREENPOS( 19, 30 ), VEC2U16( 8, 1 ), L"↑Board", ButtonStatus_HIDDEN, KeyInput_ARROW_UP );
	buttons[ButtonId_PEGS]           = button_make( SCREENPOS( 27, 30 ), VEC2U16( 7, 1 ), L"↓Pegs", ButtonStatus_HIDDEN, KeyInput_ARROW_DOWN );
	buttons[ButtonId_PLACE_SELECT]   = button_make( SCREENPOS( 35, 30 ), VEC2U16( 15, 1 ), L"↳Place/Select", ButtonStatus_HIDDEN, KeyInput_ENTER );
	buttons[ButtonId_ERASE_UNSELECT] = button_make( SCREENPOS( 50, 30 ), VEC2U16( 16, 1 ), L"Erase/Unselect", ButtonStatus_HIDDEN, KeyInput_E );
	buttons[ButtonId_VALIDATE]       = button_make( SCREENPOS( 67, 30 ), VEC2U16( 14, 1 ), L"Confirm Turn", ButtonStatus_HIDDEN, KeyInput_C );
	buttons[ButtonId_RESET]          = button_make( SCREENPOS( 81, 30 ), VEC2U16( 12, 1 ), L"Reset Turn", ButtonStatus_HIDDEN, KeyInput_R );
	buttons[ButtonId_HISTORY_UP]     = button_make( SCREENPOS( 94, 30 ), VEC2U16( 12, 1 ), L"Up History", ButtonStatus_HIDDEN, KeyInput_U );
	buttons[ButtonId_HISTORY_DOWN]   = button_make( SCREENPOS( 106, 30 ), VEC2U16( 14, 1 ), L"Down History", ButtonStatus_HIDDEN, KeyInput_D );
}

struct ComponentHeader *component_game_buttons_create( void )
{
    struct ComponentGameButtons *const comp = calloc( 1, sizeof( struct ComponentGameButtons ) );
    if ( !comp ) return NULL;

	component_make_header( &comp->header, ComponentId_GAME_BUTTONS, true );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->refreshCb = on_refresh_callback;
	callbacks->mouseMoveCb = on_mouse_move_callback;
	callbacks->inputReceivedCb = on_input_received_callback;
	callbacks->gameUpdateCb = on_game_update_callback;

	// Specific data
	init_component_data( comp );

	return (struct ComponentHeader *)comp;
}
