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


static void button_get_hovered_style( struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	*outTextStyle = STYLE( FGColor_YELLOW );
	*outKeyStyle  = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_ITALIC );
}

static void button_get_default_style( struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	*outTextStyle = STYLE( FGColor_WHITE );
	*outKeyStyle = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_ITALIC );
}

static void button_get_disabled_style( struct Style *const outTextStyle, struct Style *const outKeyStyle )
{
	*outTextStyle = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
	*outKeyStyle  = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT | Attr_ITALIC );
}


static void write_button( struct Button const *button, bool const isHovered )
{
	struct Style textStyle;
	struct Style keyStyle;

	screenpos const ul = rect_get_ul_corner( &button->box );

	cursor_update_yx( ul.y, ul.x );
	if ( button->status == ButtonStatus_ENABLED )
	{
		isHovered ? button_get_hovered_style( &textStyle, &keyStyle ) : button_get_default_style( &textStyle, &keyStyle );

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
				write_button( &boardButtons->buttons[boardButtons->hoveredButton], true );
			}
			return;
		}
	}

	if ( boardButtons->hoveredButton != ButtonId_Invalid )
	{
		write_button( &boardButtons->buttons[boardButtons->hoveredButton], false );
		boardButtons->hoveredButton = ButtonId_Invalid;
	}
}


static bool on_input_received_callback( struct ComponentHeader *header, enum KeyInput input )
{
	struct ComponentGameButtons const *comp = CAST_TO_COMP( header );
	if ( input == KeyInput_MOUSE_BTN_LEFT && comp->hoveredButton != ButtonId_Invalid )
	{
		gameloop_emit_key( comp->buttons[comp->hoveredButton].bindedKey );
		return true;
	}

	return false;
}


static void enable_callback( struct ComponentHeader *header )
{
	struct ComponentGameButtons const *comp = (struct ComponentGameButtons const *)header;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		struct Button const *button = &comp->buttons[idx];
		write_button( button, idx == comp->hoveredButton );
	}
}


static void disable_callback( struct ComponentHeader *header )
{
	struct ComponentGameButtons const *comp = (struct ComponentGameButtons const *)header;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		struct Button const *button = &comp->buttons[idx];
		rect_clear( &button->box );
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

	comp->header.id = ComponentId_GAME_BUTTONS;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
	callbacks->enableCb = enable_callback;
	callbacks->disableCb = disable_callback;
	callbacks->mouseMoveCb = on_mouse_move_callback;
	callbacks->inputReceivedCb = on_input_received_callback;

	// Specific data
	init_component_data( comp );

	return (struct ComponentHeader *)comp;
}
