#include "widgets/widget_framerate.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "fps_counter.h"
#include "mouse.h"

#include "console.h"

struct WidgetFramerate
{
    struct Widget header;
    u64           lastAverageFPS;
};


static void redraw_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_FRAMERATE );
    assert( widget->enabled );

    struct WidgetFramerate *framerate = (struct WidgetFramerate *)widget;

    console_cursor_setpos( rect_get_corner( &widget->rectBox, RectCorner_UL ) );

    console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
    console_draw( L"%3uFPS", framerate->lastAverageFPS );
    console_color_reset();
}


static void frame_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_FRAMERATE );
    struct WidgetFramerate *framerate = (struct WidgetFramerate *)widget;

    u64 const lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    if ( lastAverageFPS != framerate->lastAverageFPS )
    {
        framerate->lastAverageFPS = lastAverageFPS;
        widget->redrawNeeded = true;
    }
}


struct Widget *widget_framerate_create( void )
{
    struct WidgetFramerate *const framerate = malloc( sizeof( struct WidgetFramerate ) );
    if ( !framerate ) return NULL;

    struct Widget *const widget = &framerate->header;
    widget->id = WidgetId_FRAMERATE;
    widget->enabled = true;
	widget->redrawNeeded = true;

	widget->rectBox = rect_make( SCREENPOS( 2, 1 ), VEC2U16( 6, 1 ) );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
    callbacks->frameCb = frame_callback;
    callbacks->redrawCb = redraw_callback;

    // Specific to the widget 
    framerate->lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );

    return (struct Widget *)framerate;
}
