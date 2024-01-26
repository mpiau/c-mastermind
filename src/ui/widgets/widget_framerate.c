#include "ui/widgets.h"

#include "fps_counter.h"
#include "terminal/terminal.h"
#include "rect.h"

#include <stdlib.h>


struct WidgetFramerate
{
    struct Widget base;

    struct Rect rect;
    usize lastAverageFPS;
};


static void enable_callback( struct Widget *base )
{
    struct WidgetFramerate *widget = (struct WidgetFramerate *)base;

    widget->lastAverageFPS = 0;
}


static void disable_callback( struct Widget *base )
{
    struct WidgetFramerate *widget = (struct WidgetFramerate *)base;

    rect_clear( &widget->rect );
}


static void frame_callback( struct Widget *base )
{
    struct WidgetFramerate *widget = (struct WidgetFramerate *)base;

    usize const lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    if ( lastAverageFPS != widget->lastAverageFPS )
    {
        cursor_update_pos( rect_get_ul_corner( &widget->rect ) );
        style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
        term_write( L"%3uFPS", lastAverageFPS );

        widget->lastAverageFPS = lastAverageFPS;
    }
}


struct Widget *widget_framerate_create( void )
{
    struct WidgetFramerate *const widget = calloc( 1, sizeof( struct WidgetFramerate ) );
    if ( !widget ) return NULL;

    widget->base.name = "Framerate";
    widget->base.enabledScenes = UIScene_ALL;

    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;
    widget->base.frameCb = frame_callback;

    // Widget specific

    widget->rect = rect_make( SCREENPOS( 1, 1 ), VEC2U16( 7, 1 ) );

    return (struct Widget *)widget;
}
