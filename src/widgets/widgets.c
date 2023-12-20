#include "widgets/widgets.h"
#include "widgets/widget_definition.h"

#include "widgets/widget_timer.h"

static struct Widget *s_widgets[WidgetId_Count] = {};


// Array with the list of ID for the priority frame / priority input


inline static
bool widget_exists( enum WidgetId const id )
{
    return s_widgets[id] != NULL;
}


bool widgets_hook( struct Widget *const widget )
{
    assert( widget );

    if ( widget_exists( widget->id ) )
    {
        return false;
    }

    s_widgets[widget->id] = widget;
    widget->references += 1;
    return true;
}


void widgets_unhook( struct Widget const *const widget )
{
    assert( widget );

    struct Widget *hookedWidget = s_widgets[widget->id];
    if ( hookedWidget )
    {
        hookedWidget->references -= 1;
        hookedWidget = NULL;
    }
}


bool widgets_is_hook( struct Widget const *const widget )
{
    return widget_exists( widget->id );
}


// /////////////////////////////////////////////////////////


void widgets_frame( void )
{

}


void widgets_hide( void )
{
}