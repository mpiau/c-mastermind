#include "ui/widgets.h"

#include "terminal/terminal.h"
#include "rect.h"
#include "events.h"

#include <stdlib.h>


struct WidgetMousePos
{
    struct Widget base;

    struct Rect rect;
};


static void draw_mouse_pos( struct WidgetMousePos *widget, screenpos const pos )
{
	cursor_update_pos( rect_get_ul_corner( &widget->rect ) );
	style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
    term_write( L"M:%ux%u  ", pos.x, pos.y );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    if ( event->type == EventType_MOUSE_MOVED )
    {
        draw_mouse_pos( (struct WidgetMousePos *)subscriber, event->mouseMoved.pos );
    }

    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
    struct WidgetMousePos *widget = (struct WidgetMousePos *)base;
    draw_mouse_pos( widget, SCREENPOS( 0, 0 ) );
}


static void disable_callback( struct Widget *base )
{
    struct WidgetMousePos *widget = (struct WidgetMousePos *)base;
    rect_clear( &widget->rect );
}


struct Widget *widget_mousepos_create( void )
{
    struct WidgetMousePos *const widget = calloc( 1, sizeof( struct WidgetMousePos ) );
    if ( !widget ) return NULL;

    widget->base.name = "MousePosition";
    widget->base.enabledScenes = UIScene_ALL;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    // Widget specific

    event_register( (struct Widget *)widget, on_event_callback );
    event_subscribe( (struct Widget *)widget, EventType_MOUSE_MOVED );

    widget->rect = rect_make( SCREENPOS( 41, 1 ), VEC2U16( 11, 1 ) );

    return (struct Widget *)widget;
}
