#include "widgets/widget_board_buttons.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

struct WidgetBoardButtons
{
	struct Widget header;
};


static void redraw_callback( struct Widget *widget )
{
	screenpos ul  = widget->box.contentUpLeft;
	console_cursor_setpos( ul );
	console_draw( L"VALIDER   RESET ROW   NEW GAME   QUIT TO DESKTOP" );
}

struct Widget *widget_board_buttons_create( void )
{
    struct WidgetBoardButtons *const buttons = malloc( sizeof( struct WidgetBoardButtons ) );
    if ( !buttons ) return NULL;

	struct Widget *const widget = &buttons->header;

    widget->id = WidgetID_BOARD_BUTTONS;
	widget->visibilityStatus = WidgetVisibilityStatus_VISIBLE;

	assert( widget_exists( WidgetId_FRAMERATE ) );
	struct WidgetBox const *framerate = &widget_optget( WidgetId_FRAMERATE )->box;

    screenpos const borderUpLeft = (screenpos) { .x = framerate->borderBottomRight.x + 40, .y = framerate->borderUpLeft.y };
    screenpos const contentSize  = (vec2u16)   { .x = 70, .y = 1 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
//    callbacks->frameCb = frame_callback;
    callbacks->redrawCb = redraw_callback;

	return (struct Widget *)buttons;
}