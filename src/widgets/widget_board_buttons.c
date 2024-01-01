#include "widgets/widget_board_buttons.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "game.h"
#include "keyboard_inputs.h"

enum Buttons
{
	Button_NEW_GAME,
	Button_PAUSE,
	Button_RESUME,
	Button_SETTINGS,
	Button_QUIT,

	Button_Count,

	// Special edge case, impl only.
	Button_NONE = Button_Count
};

struct WidgetBoardButtons
{
	struct Widget header;

	struct Rect  buttons[Button_Count];
	utf16 const  *buttonText[Button_Count];
	enum Buttons hoveredButton;
};


static void mouse_move_callback( struct Widget *widget, screenpos oldPos, screenpos newPos )
{
	struct WidgetBoardButtons *buttons = (struct WidgetBoardButtons *)widget;

	for ( enum Buttons idx = 0; idx < Button_Count; ++idx )
	{
		if ( rect_is_inside( &buttons->buttons[idx], newPos ) )
		{
			if ( buttons->hoveredButton != idx )
			{
				buttons->hoveredButton = idx;
				widget->redrawNeeded = true;
			}
			return;
		}
	}

	if ( buttons->hoveredButton != Button_NONE )
	{
		buttons->hoveredButton = Button_NONE;
		widget->redrawNeeded = true;
	}
}


static void mouse_click_callback( struct Widget *widget, screenpos clickPos, enum KeyInput mouseKey )
{
	// If button clicked -> Do the relative thing. (Emit input ?)
}


static void redraw_callback( struct Widget *widget )
{
	struct WidgetBoardButtons *buttons = (struct WidgetBoardButtons *)widget;

	enum ConsoleColorFG const defaultTextColor = ConsoleColorFG_BRIGHT_BLACK;
	enum ConsoleColorFG const defaultKeyColor = ConsoleColorFG_YELLOW;

	enum ConsoleColorFG const hoveredTextColor = ConsoleColorFG_WHITE;
	enum ConsoleColorFG const hoveredKeyColor = ConsoleColorFG_BRIGHT_YELLOW;

	for ( enum Buttons idx = 0; idx < Button_Count; ++idx )
	{
		screenpos const ul = rect_get_corner( &buttons->buttons[idx], RectCorner_UL );
		bool const isHovered = ( buttons->hoveredButton == idx );
		utf16 const *text = buttons->buttonText[idx];

		console_cursor_setpos( ul );

		console_color_fg( isHovered ? hoveredTextColor : defaultTextColor );
		console_draw( L"[" );

		console_color_fg( isHovered ? hoveredKeyColor : defaultKeyColor );
		console_draw( L"%lc", text[0] );

		console_color_fg( isHovered ? hoveredTextColor : defaultTextColor );
		console_draw( L"%S]", &text[1] );
	}
}


struct Widget *widget_board_buttons_create( void )
{
    struct WidgetBoardButtons *const buttons = malloc( sizeof( struct WidgetBoardButtons ) );
    if ( !buttons ) return NULL;

	struct Widget *const widget = &buttons->header;
    widget->id = WidgetId_BOARD_BUTTONS;
	widget->enabled = true;
	widget->redrawNeeded = true;

	widget->rectBox = rect_make( SCREENPOS( 69, 1 ), VEC2U16( 50, 1 ) );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
    callbacks->redrawCb = redraw_callback;
	callbacks->mouseMoveCb = mouse_move_callback;

	// Specific data

	buttons->hoveredButton = Button_NONE;
	buttons->buttons[Button_NEW_GAME]    = rect_make( SCREENPOS( 68, 1 ), VEC2U16( 10, 1 ) ); // "[New Game]" = 10 length, + 3 each time for spaces
	buttons->buttonText[Button_NEW_GAME] = L"New Game";

	buttons->buttons[Button_PAUSE]    = rect_make( SCREENPOS( 81, 1 ), VEC2U16( 7, 1 ) ); // "[Pause]" = 7 length
	buttons->buttonText[Button_PAUSE] = L"Pause";

	buttons->buttons[Button_RESUME]    = rect_make( SCREENPOS( 91, 1 ), VEC2U16( 8, 1 ) ); // "[Resume]" = 8 length
	buttons->buttonText[Button_RESUME] = L"Resume";

	buttons->buttons[Button_SETTINGS]    = rect_make( SCREENPOS( 102, 1 ), VEC2U16( 10, 1 ) ); // "[Settings]" = 10 length
	buttons->buttonText[Button_SETTINGS] = L"Settings";

	buttons->buttons[Button_QUIT]    = rect_make( SCREENPOS( 115, 1 ), VEC2U16( 6, 1 ) ); // "[Quit]" = 6 length
	buttons->buttonText[Button_QUIT] = L"Quit";

	return (struct Widget *)buttons;
}