#include "widgets/widget_screen_size.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "console.h"
#include "console_screen.h"
#include "game.h"

struct WidgetScreenSize
{
    struct Widget header;
};


static void redraw_callback( struct Widget *widget )
{
    console_cursor_setpos( rect_get_corner( &widget->rectBox, RectCorner_UL ) );

	vec2u16 const size = console_screen_get_size();

	enum ConsoleColorFG const defaultColor = ConsoleColorFG_BRIGHT_BLACK;
	enum ConsoleColorFG const errorColor = ConsoleColorFG_RED;

	enum ConsoleColorFG const widthColor = console_screen_is_width_too_small() ? errorColor : defaultColor;
	enum ConsoleColorFG const heightColor = console_screen_is_height_too_small() ? errorColor : defaultColor;

	u32 charsDrawn = 0;

	console_color_fg( widthColor );
	charsDrawn = console_draw( L"%u", size.w );

	console_color_fg( defaultColor );
	charsDrawn += console_draw( L"x" );

	console_color_fg( heightColor );
	charsDrawn += console_draw( L"%u", size.h );

	if ( heightColor == errorColor || widthColor == errorColor )
	{
		console_color_fg( errorColor );
		charsDrawn += console_draw( L" (req: %ux%u)", GAME_SIZE_WIDTH, GAME_SIZE_HEIGHT );
	}

	// Fill the rest of the widget with spaces to remove the unwanted chars in screen
	console_draw( L"%*lc", rect_get_width( &widget->rectBox ) - charsDrawn, L' ' );
	
	console_color_reset();
}


static void on_resize_callback( struct Widget *widget, vec2u16 const oldSize, vec2u16 const newSize )
{
	widget->redrawNeeded = true;
}


struct Widget *widget_screen_size_create( void )
{
    struct WidgetScreenSize *const screenSize = malloc( sizeof( struct WidgetScreenSize ) );
    if ( !screenSize ) return NULL;
	memset( screenSize, 0, sizeof( *screenSize ) );

    struct Widget *const widget = &screenSize->header;
    widget->id = WidgetId_SCREEN_SIZE;
    widget->enabled = true;
	widget->redrawNeeded = true;

	// Place the widget relative to the framerate widget. This way, increase the framerate widget size
	// won't overlap on this one
	assert( widget_exists( WidgetId_FRAMERATE ) );
	struct Widget const *framerate = widget_optget( WidgetId_FRAMERATE );
	screenpos const framerateBR = rect_get_corner( &framerate->rectBox, RectCorner_BR );
	widget->rectBox = rect_make( SCREENPOS( framerateBR.x + 2, 1 ), VEC2U16( 22, 1 ) );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
	callbacks->redrawCb = redraw_callback;
	callbacks->resizeCb = on_resize_callback;
	callbacks->frameCb = NULL;

    return (struct Widget *)screenSize;
}
