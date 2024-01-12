#include "components/component_game_buttons.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "game.h"
#include "keyboard_inputs.h"
#include "gameloop.h"

#include "terminal/terminal_screen.h"
#include "terminal/terminal_style.h"

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


static void button_get_hovered_color( enum ButtonId const id, enum ConsoleColorFG *const outTextColor, enum ConsoleColorFG *const outKeyColor )
{
	switch( id )
	{
		case ButtonId_VALIDATE:
			*outTextColor = ConsoleColorFG_GREEN;
			*outKeyColor = ConsoleColorFG_BRIGHT_GREEN;
			break;

		case ButtonId_ABANDON_GAME:
		case ButtonId_RESET:
			*outTextColor = ConsoleColorFG_RED;
			*outKeyColor = ConsoleColorFG_BRIGHT_RED;
			break;

		default:
			*outTextColor = ConsoleColorFG_YELLOW;
			*outKeyColor = ConsoleColorFG_BRIGHT_YELLOW;
			break;
	}
}

static void button_get_default_color( enum ButtonId const id, enum ConsoleColorFG *const outTextColor, enum ConsoleColorFG *const outKeyColor )
{
	switch( id )
	{
		case ButtonId_VALIDATE:
			*outKeyColor = ConsoleColorFG_GREEN;
			break;

		case ButtonId_ABANDON_GAME:
		case ButtonId_RESET:
			*outKeyColor = ConsoleColorFG_RED;
			break;

		default:
			*outKeyColor = ConsoleColorFG_YELLOW;
			break;
	}

	*outTextColor = ConsoleColorFG_BRIGHT_BLACK;
}

static void button_get_disabled_color( enum ConsoleColorFG *const outTextColor, enum ConsoleColorFG *const outKeyColor )
{
	*outTextColor = ConsoleColorFG_BRIGHT_BLACK;
	*outKeyColor = ConsoleColorFG_BRIGHT_BLACK;
}


static void game_buttons_update_status( struct ComponentHeader *header, enum ButtonStatus const status )
{
	struct ComponentGameButtons *widget = (struct ComponentGameButtons *)header;
	screenpos const mousePosition = mouse_get_position();

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


static void mouse_move_callback( struct ComponentHeader *const widget, screenpos const pos )
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


static void game_update_callback( struct ComponentHeader *widget, struct Mastermind const *mastermind, enum GameUpdateType type )
{
	struct ComponentGameButtons *boardButtons = (struct ComponentGameButtons *)widget;

	if ( type == GameUpdateType_GAME_FINISHED )
	{
		struct Button *abandonButton = &boardButtons->buttons[ButtonId_ABANDON_GAME];
		abandonButton->status = ButtonStatus_DISABLED;
		if ( boardButtons->hoveredButton == ButtonId_ABANDON_GAME )
		{
			boardButtons->hoveredButton = ButtonId_Invalid;
		}
		game_buttons_update_status( widget, ButtonStatus_HIDDEN );
		widget->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_GAME_NEW )
	{
		struct Button *abandonButton = &boardButtons->buttons[ButtonId_ABANDON_GAME];
		abandonButton->status = ButtonStatus_ENABLED;
		if ( rect_is_inside( &abandonButton->box, mouse_get_position() ) )
		{
			boardButtons->hoveredButton = ButtonId_ABANDON_GAME;
		}
		game_buttons_update_status( widget, ButtonStatus_ENABLED );
		widget->refreshNeeded = true;
	}
}


static void mouse_click_callback( struct ComponentHeader *widget, screenpos clickPos, enum MouseButton mouseButton )
{
	struct ComponentGameButtons *buttons = CAST_TO_COMPONENT( widget );

	if ( mouseButton == MouseButton_LEFT_CLICK && buttons->hoveredButton != ButtonId_Invalid )
	{
		gameloop_emit_key( buttons->buttons[buttons->hoveredButton].bindedKey );
	}
}


static void redraw_callback( struct ComponentHeader *widget )
{
	struct ComponentGameButtons *boardButtons = (struct ComponentGameButtons *)widget;

	enum ConsoleColorFG textColor;
	enum ConsoleColorFG keyColor;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		struct Button *button = &boardButtons->buttons[idx];

		screenpos const ul = rect_get_corner( &button->box, RectCorner_UL );
		bool const isHovered = ( boardButtons->hoveredButton == idx );

		console_cursor_setpos( ul );
		if ( button->status == ButtonStatus_ENABLED )
		{
			isHovered ? button_get_hovered_color( idx, &textColor, &keyColor ) : button_get_default_color( idx, &textColor, &keyColor );

			console_draw( L"\x1b[0m" );
			console_color_fg( textColor );
			console_draw( L"[" );

			console_color_fg( keyColor );
			console_draw( L"%lc", button->text[0] );

			console_color_fg( textColor );
			console_draw( L"%S]", &button->text[1] );
		}
		else if ( button->status == ButtonStatus_DISABLED )
		{
			button_get_disabled_color( &textColor, &keyColor );
			console_draw( L"\x1b[2m" );
			console_color_fg( textColor );
			console_draw( L"[%S]", button->text );
		}
		else
		{
			console_draw( L"%*lc", wcslen( button->text ) + 2, L' ' ); // + 2 for '[' and ']'
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
    callbacks->redrawCb = redraw_callback;
	callbacks->mouseMoveCb = mouse_move_callback;
	callbacks->mouseClickCb = mouse_click_callback;
	callbacks->gameUpdateCb = game_update_callback;

	// Specific data
	init_component_data( comp );

	return (struct ComponentHeader *)comp;
}
