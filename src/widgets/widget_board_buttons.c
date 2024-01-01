#include "widgets/widget_board_buttons.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "game.h"
#include "keyboard_inputs.h"
#include "gameloop.h"

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

	ButtonId_Count,

	// Special edge case, impl only.
	ButtonId_Invalid = ButtonId_Count
};

struct Button
{
	struct Rect box;
	utf16 const *text;
};


struct WidgetBoardButtons
{
	struct Widget header;

	struct Button buttons[ButtonId_Count];
	enum ButtonId hoveredButton;
};


static void button_get_hovered_color( enum ButtonId id, enum ConsoleColorFG *outTextColor, enum ConsoleColorFG *outKeyColor )
{
	switch( id )
	{
		case ButtonId_VALIDATE:
			*outTextColor = ConsoleColorFG_GREEN;
			*outKeyColor = ConsoleColorFG_BRIGHT_GREEN;
			break;
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

static void button_get_default_color( enum ButtonId id, enum ConsoleColorFG *outTextColor, enum ConsoleColorFG *outKeyColor )
{
	switch( id )
	{
		case ButtonId_VALIDATE:
			*outKeyColor = ConsoleColorFG_GREEN;
			break;
		case ButtonId_RESET:
			*outKeyColor = ConsoleColorFG_RED;
			break;

		default:
			*outKeyColor = ConsoleColorFG_YELLOW;
			break;
	}

	*outTextColor = ConsoleColorFG_BRIGHT_BLACK;
}

static void button_get_disabled_color( enum ConsoleColorFG *outTextColor, enum ConsoleColorFG *outKeyColor )
{
	*outTextColor = ConsoleColorFG_BRIGHT_BLACK;
	*outKeyColor = ConsoleColorFG_BRIGHT_BLACK;
}


static void mouse_move_callback( struct Widget *widget, screenpos oldPos, screenpos newPos )
{
	struct WidgetBoardButtons *buttons = (struct WidgetBoardButtons *)widget;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		if ( rect_is_inside( &buttons->buttons[idx].box, newPos ) )
		{
			if ( buttons->hoveredButton != idx )
			{
				buttons->hoveredButton = idx;
				widget->redrawNeeded = true;
			}
			return;
		}
	}

	if ( buttons->hoveredButton != ButtonId_Invalid )
	{
		buttons->hoveredButton = ButtonId_Invalid;
		widget->redrawNeeded = true;
	}
}


static void mouse_click_callback( struct Widget *widget, screenpos clickPos, enum MouseButton mouseButton )
{
	if ( mouseButton != MouseButton_LeftClick ) return; // Only accept LeftClick here

	struct WidgetBoardButtons *buttons = (struct WidgetBoardButtons *)widget;
	if ( buttons->hoveredButton == ButtonId_Invalid ) return;

	switch( buttons->hoveredButton )
	{
		case ButtonId_NEW_GAME:
			gameloop_emit_key( KeyInput_N );
			break;
		case ButtonId_ABANDON_GAME:
			gameloop_emit_key( KeyInput_A );
			break;
		case ButtonId_GAME_RULES:
			gameloop_emit_key( KeyInput_G );
			break;
		case ButtonId_SETTINGS:
			gameloop_emit_key( KeyInput_S );
			break;
		case ButtonId_QUIT:
			gameloop_emit_key( KeyInput_Q );
			break;
		case ButtonId_PREVIOUS:
			gameloop_emit_key( KeyInput_ARROW_LEFT );
			break;
		case ButtonId_NEXT:
			gameloop_emit_key( KeyInput_ARROW_RIGHT );
			break;
		case ButtonId_BOARD:
			gameloop_emit_key( KeyInput_ARROW_UP );
			break;
		case ButtonId_PEGS:
			gameloop_emit_key( KeyInput_ARROW_DOWN );
			break;
		case ButtonId_PLACE_SELECT:
			gameloop_emit_key( KeyInput_P );
			break;
		case ButtonId_ERASE_UNSELECT:
			gameloop_emit_key( KeyInput_E );
			break;
		case ButtonId_VALIDATE:
			gameloop_emit_key( KeyInput_V );
			break;
		case ButtonId_RESET:
			gameloop_emit_key( KeyInput_R );
			break;
		case ButtonId_HISTORY_UP:
			gameloop_emit_key( KeyInput_U );
			break;
		case ButtonId_HISTORY_DOWN:
			gameloop_emit_key( KeyInput_D );
			break;
	}

	// After a successfull click, reset the hovered parameter.
	// This way, the button won't be highlighted anymore after a click, which seems better in a UX perspective
	// buttons->hoveredButton = ButtonId_Invalid;
	// widget->redrawNeeded = true;

	// Well it was good until I tested clicking multiple times on the same button without moving the mouse.
	// Once disabled, there is no feedback for the user.
	// TODO: Either we put the button in black bright on click and then reapply the color, or not remove the color at all
}


static void redraw_callback( struct Widget *widget )
{
	struct WidgetBoardButtons *boardButtons = (struct WidgetBoardButtons *)widget;

	enum ConsoleColorFG textColor;
	enum ConsoleColorFG keyColor;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		struct Button *button = &boardButtons->buttons[idx];
		screenpos const ul = rect_get_corner( &button->box, RectCorner_UL );
		bool const isHovered = ( boardButtons->hoveredButton == idx );

		isHovered ? button_get_hovered_color( idx, &textColor, &keyColor ) : button_get_default_color( idx, &textColor, &keyColor );

		console_cursor_setpos( ul );

		console_color_fg( textColor );
		console_draw( L"[" );

		console_color_fg( keyColor );
		console_draw( L"%lc", button->text[0] );

		console_color_fg( textColor );
		console_draw( L"%S]", &button->text[1] );
	}
}


struct Widget *widget_board_buttons_create( void )
{
    struct WidgetBoardButtons *const boardButtons = malloc( sizeof( struct WidgetBoardButtons ) );
    if ( !boardButtons ) return NULL;

	struct Widget *const widget = &boardButtons->header;
    widget->id = WidgetId_BOARD_BUTTONS;
	widget->enabled = true;
	widget->redrawNeeded = true;

	// Still useful ?
	widget->rectBox = rect_make( SCREENPOS( 69, 1 ), VEC2U16( 50, 1 ) );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
    callbacks->redrawCb = redraw_callback;
	callbacks->mouseMoveCb = mouse_move_callback;
	callbacks->mouseClickCb = mouse_click_callback;

	// Specific data

	// Upper row

	struct Button *button = &boardButtons->buttons[ButtonId_NEW_GAME];
	button->box = rect_make( SCREENPOS( 55, 1 ), VEC2U16( 10, 1 ) );
	button->text = L"New Game";
	
	button = &boardButtons->buttons[ButtonId_ABANDON_GAME];
	button->box = rect_make( SCREENPOS( 68, 1 ), VEC2U16( 15, 1 ) );
	button->text = L"Abandon Game";

	button = &boardButtons->buttons[ButtonId_GAME_RULES];
	button->box = rect_make( SCREENPOS( 86, 1 ), VEC2U16( 12, 1 ) );
	button->text = L"Game Rules";

	button = &boardButtons->buttons[ButtonId_SETTINGS];
	button->box = rect_make( SCREENPOS( 101, 1 ), VEC2U16( 10, 1 ) );
	button->text = L"Settings";

	button = &boardButtons->buttons[ButtonId_QUIT];
	button->box = rect_make( SCREENPOS( 114, 1 ), VEC2U16( 6, 1 ) );
	button->text = L"Quit";

	// Bottom row

//	L"[←Prev][→Next] [↑Board][↓Pegs] [↳Place/Select][Erase/Unselect] [Confirm Turn][Reset Turn] [Up History][Down History]"
	button = &boardButtons->buttons[ButtonId_PREVIOUS];
	button->box = rect_make( SCREENPOS( 3, 30 ), VEC2U16( 7, 1 ) );
	button->text = L"←Prev";

	button = &boardButtons->buttons[ButtonId_NEXT];
	button->box = rect_make( SCREENPOS( 11, 30 ), VEC2U16( 7, 1 ) );
	button->text = L"→Next";

	button = &boardButtons->buttons[ButtonId_BOARD];
	button->box = rect_make( SCREENPOS( 19, 30 ), VEC2U16( 8, 1 ) );
	button->text = L"↑Board";

	button = &boardButtons->buttons[ButtonId_PEGS];
	button->box = rect_make( SCREENPOS( 27, 30 ), VEC2U16( 7, 1 ) );
	button->text = L"↓Pegs";

	button = &boardButtons->buttons[ButtonId_PLACE_SELECT];
	button->box = rect_make( SCREENPOS( 35, 30 ), VEC2U16( 15, 1 ) );
	button->text = L"↳Place/Select";

	button = &boardButtons->buttons[ButtonId_ERASE_UNSELECT];
	button->box = rect_make( SCREENPOS( 50, 30 ), VEC2U16( 16, 1 ) );
	button->text = L"Erase/Unselect";

	button = &boardButtons->buttons[ButtonId_VALIDATE];
	button->box = rect_make( SCREENPOS( 67, 30 ), VEC2U16( 14, 1 ) );
	button->text = L"Confirm Turn";

	button = &boardButtons->buttons[ButtonId_RESET];
	button->box = rect_make( SCREENPOS( 81, 30 ), VEC2U16( 12, 1 ) );
	button->text = L"Reset Turn";

	button = &boardButtons->buttons[ButtonId_HISTORY_UP];
	button->box = rect_make( SCREENPOS( 94, 30 ), VEC2U16( 12, 1 ) );
	button->text = L"Up History";

	button = &boardButtons->buttons[ButtonId_HISTORY_DOWN];
	button->box = rect_make( SCREENPOS( 106, 30 ), VEC2U16( 14, 1 ) );
	button->text = L"Down History";
	
	boardButtons->hoveredButton = ButtonId_Invalid;

	return (struct Widget *)boardButtons;
}