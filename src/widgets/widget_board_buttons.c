#include "widgets/widget_board_buttons.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "game.h"
#include "keyboard_inputs.h"
#include "gameloop.h"

enum ButtonId
{
	ButtonId_NEW_GAME,
	ButtonId_RULES,
	ButtonId_SETTINGS,
	ButtonId_QUIT,

	ButtonId_Count,

	// Special edge case, impl only.
	ButtonId_INVALID = ButtonId_Count
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

	if ( buttons->hoveredButton != ButtonId_INVALID )
	{
		buttons->hoveredButton = ButtonId_INVALID;
		widget->redrawNeeded = true;
	}
}


static void mouse_click_callback( struct Widget *widget, screenpos clickPos, enum MouseButton mouseButton )
{
	if ( mouseButton != MouseButton_LeftClick ) return; // Only accept LeftClick here

	struct WidgetBoardButtons *buttons = (struct WidgetBoardButtons *)widget;

	switch( buttons->hoveredButton )
	{
		case ButtonId_NEW_GAME:
			gameloop_emit_key( KeyInput_N );
			break;
		case ButtonId_RULES:
			gameloop_emit_key( KeyInput_R );
			break;
		case ButtonId_SETTINGS:
			gameloop_emit_key( KeyInput_S );
			break;
		case ButtonId_QUIT:
			gameloop_emit_key( KeyInput_Q );
			break;

		default: return;
	}

	// After a successfull click (default return early), reset the hovered parameter.
	// This way, the button won't be highlighted anymore after a click, which seems better in a UX perspective
	buttons->hoveredButton = ButtonId_INVALID;
	widget->redrawNeeded = true;
}


static void redraw_callback( struct Widget *widget )
{
	struct WidgetBoardButtons *boardButtons = (struct WidgetBoardButtons *)widget;

	enum ConsoleColorFG const defaultTextColor = ConsoleColorFG_BRIGHT_BLACK;
	enum ConsoleColorFG const defaultKeyColor = ConsoleColorFG_YELLOW;

	enum ConsoleColorFG const hoveredTextColor = ConsoleColorFG_WHITE;
	enum ConsoleColorFG const hoveredKeyColor = ConsoleColorFG_BRIGHT_YELLOW;

	for ( enum ButtonId idx = 0; idx < ButtonId_Count; ++idx )
	{
		struct Button *button = &boardButtons->buttons[idx];
		screenpos const ul = rect_get_corner( &button->box, RectCorner_UL );
		bool const isHovered = ( boardButtons->hoveredButton  == idx );

		console_cursor_setpos( ul );

		console_color_fg( isHovered ? hoveredTextColor : defaultTextColor );
		console_draw( L"[" );

		console_color_fg( isHovered ? hoveredKeyColor : defaultKeyColor );
		console_draw( L"%lc", button->text[0] );

		console_color_fg( isHovered ? hoveredTextColor : defaultTextColor );
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

	widget->rectBox = rect_make( SCREENPOS( 69, 1 ), VEC2U16( 50, 1 ) );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
    callbacks->redrawCb = redraw_callback;
	callbacks->mouseMoveCb = mouse_move_callback;
	callbacks->mouseClickCb = mouse_click_callback;

	// Specific data

	struct Button *button = &boardButtons->buttons[ButtonId_NEW_GAME];
	button->box = rect_make( SCREENPOS( 78, 1 ), VEC2U16( 10, 1 ) );
	button->text = L"New Game";
	
	button = &boardButtons->buttons[ButtonId_RULES];
	button->box = rect_make( SCREENPOS( 91, 1 ), VEC2U16( 7, 1 ) );
	button->text = L"Rules";

	button = &boardButtons->buttons[ButtonId_SETTINGS];
	button->box = rect_make( SCREENPOS( 101, 1 ), VEC2U16( 10, 1 ) );
	button->text = L"Settings";

	button = &boardButtons->buttons[ButtonId_QUIT];
	button->box = rect_make( SCREENPOS( 114, 1 ), VEC2U16( 6, 1 ) );
	button->text = L"Quit";

	boardButtons->hoveredButton = ButtonId_INVALID;

	return (struct Widget *)boardButtons;
}