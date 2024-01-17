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
	ButtonId_GAME_RULES,
	ButtonId_SETTINGS,
	ButtonId_QUIT,

	ButtonId_Count,

	// Special edge case, impl only.
	ButtonId_Invalid = ButtonId_Count
};

/*enum ButtonType
{
	ButtonType_ONE_LINE,
	ButtonType_TWO_LINE
};*/

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
#define CAST_TO_COMP( _header ) ( ( struct ComponentGameButtons * )( _header ) )


static void button_get_hovered_style( enum ButtonId const id, struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	switch( id )
	{
		/*case ButtonId_VALIDATE:
			*outTextStyle = STYLE( FGColor_GREEN );
			*outKeyStyle  = STYLE( FGColor_BRIGHT_GREEN );
			break;*/

//		case ButtonId_SOLUTION:
//		case ButtonId_RESET:
//			*outTextStyle = STYLE( FGColor_BRIGHT_RED );
//			*outKeyStyle  = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT | Attr_ITALIC );
//			break;

		default:
			*outTextStyle = STYLE( FGColor_YELLOW );
			*outKeyStyle  = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_ITALIC );
			break;
	}
}

static void button_get_default_style( enum ButtonId const id, struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	switch( id )
	{
		/*case ButtonId_VALIDATE:
			*outKeyStyle = STYLE( FGColor_GREEN );
			break;*/

//		case ButtonId_SOLUTION:
//		case ButtonId_RESET:
//			*outTextStyle = STYLE( FGColor_RED );
//			break;

		default:
			*outTextStyle = STYLE( FGColor_WHITE );
			break;
	}

	*outKeyStyle = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_ITALIC );
}

static void button_get_disabled_style( struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	*outTextStyle = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
	*outKeyStyle  = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT | Attr_ITALIC );
}


static void on_mouse_move_callback( struct ComponentHeader *const header, screenpos const pos )
{
	struct ComponentGameButtons *boardButtons = (struct ComponentGameButtons *)header;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		if ( boardButtons->buttons[idx].status == ButtonStatus_ENABLED && rect_is_inside( &boardButtons->buttons[idx].box, pos ) )
		{
			if ( boardButtons->hoveredButton != idx )
			{
				boardButtons->hoveredButton = idx;
				header->refreshNeeded = true;
			}
			return;
		}
	}

	if ( boardButtons->hoveredButton != ButtonId_Invalid )
	{
		boardButtons->hoveredButton = ButtonId_Invalid;
		header->refreshNeeded = true;
	}
}


static void on_game_update_callback( struct ComponentHeader *header, enum GameUpdateType type )
{
	struct ComponentGameButtons *boardButtons = (struct ComponentGameButtons *)header;

	if ( type == GameUpdateType_GAME_FINISHED )
	{
//		struct Button *abandonButton = &boardButtons->buttons[ButtonId_SOLUTION];
//		abandonButton->status = ButtonStatus_DISABLED;
//		if ( boardButtons->hoveredButton == ButtonId_SOLUTION )
//		{
//			boardButtons->hoveredButton = ButtonId_Invalid;
//		}
		header->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_GAME_NEW )
	{
//		struct Button *abandonButton = &boardButtons->buttons[ButtonId_SOLUTION];
//		abandonButton->status = ButtonStatus_ENABLED;
//		if ( rect_is_inside( &abandonButton->box, mouse_pos() ) )
//		{
//			boardButtons->hoveredButton = ButtonId_SOLUTION;
//		}
		header->refreshNeeded = true;
	}
}


static bool on_input_received_callback( struct ComponentHeader *header, enum KeyInput input )
{
	struct ComponentGameButtons *buttons = CAST_TO_COMP( header );
	if ( input == KeyInput_MOUSE_BTN_LEFT && buttons->hoveredButton != ButtonId_Invalid )
	{
		gameloop_emit_key( buttons->buttons[buttons->hoveredButton].bindedKey );
		return true;
	}

	return false;
}


static void on_refresh_callback( struct ComponentHeader const *header )
{
	struct ComponentGameButtons const *comp = (struct ComponentGameButtons const *)header;

	struct Style textStyle;
	struct Style keyStyle;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		struct Button const *button = &comp->buttons[idx];

		screenpos const ul = rect_get_ul_corner( &button->box );
		bool const isHovered = ( comp->hoveredButton == idx );

		cursor_update_yx( ul.y, ul.x );
		if ( button->status == ButtonStatus_ENABLED )
		{
			isHovered ? button_get_hovered_style( idx, &textStyle, &keyStyle ) : button_get_default_style( idx, &textStyle, &keyStyle );

			style_update( textStyle );
			term_write( L"%S", button->text );

			cursor_update_yx( ul.y + 1, ul.x );
			style_update( keyStyle );
			term_write( L"[%s]", key_input_get_name( button->bindedKey ) );
		}
		else if ( button->status == ButtonStatus_DISABLED )
		{
			button_get_disabled_style( &textStyle, &keyStyle );

			style_update( textStyle );
			term_write( L"%S", button->text );

			cursor_update_yx( ul.y + 1, ul.x );
			style_update( keyStyle );
			term_write( L"[%s]", key_input_get_name( button->bindedKey ) );
		}
/*		else
		{
			term_write( L"%s", button->text );
			term_write( L"%*lc", wcslen( button->text ) + 2, L' ' ); // + 2 for '[' and ']'
		}*/
	}
}


static inline struct Button button_make( screenpos const ul, utf16 const *const text, enum ButtonStatus const status, enum KeyInput const bindedKey )
{
	assert( text != NULL );

	vec2u16 const buttonSize = VEC2U16( wcslen( text ), 2 );

	return (struct Button) {
		.box = rect_make( ul, buttonSize ),
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
	buttons[ButtonId_NEW_GAME]   = button_make( SCREENPOS( 84, 29 ), L"New Game", ButtonStatus_ENABLED, KeyInput_SPACE );
	buttons[ButtonId_GAME_RULES] = button_make( SCREENPOS( 94, 29 ), L"Game Rules", ButtonStatus_DISABLED, KeyInput_T );
	buttons[ButtonId_SETTINGS]   = button_make( SCREENPOS( 106, 29 ), L"Settings", ButtonStatus_DISABLED, KeyInput_S );
	buttons[ButtonId_QUIT]       = button_make( SCREENPOS( 116, 29 ), L"Quit", ButtonStatus_ENABLED, KeyInput_ESCAPE );
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
