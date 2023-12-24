#include "widgets/widgets.h"
#include "widgets/widget_definition.h"

#include "widgets/widget_timer.h"

static struct Widget *s_widgets[WidgetId_Count] = {};


// Array with the list of ID for the priority frame / priority input


static
void widgets_on_mouse_mouvement( screenpos const oldPos, screenpos const newPos )
{
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget && widget->mouseMouvementCallback )
        {
            widget->mouseMouvementCallback( oldPos, newPos );
        }
    }
}


inline static
bool widget_exists( enum WidgetId const id )
{
    return s_widgets[id] != NULL;
}


bool widgets_init( void )
{
	mouse_register_on_mouse_mouvement_callback( widgets_on_mouse_mouvement );
	return true;
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
    // Need to define an order of frame, because some popups can be put on top of the others
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget && widget->frameCallback )
        {
            widget->frameCallback();
        }
    }
}


void widgets_hide( void )
{
}