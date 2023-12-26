#include "widgets/widget.h"
#include "widgets/widget_definition.h"

#include "widgets/widget_utils.h"
#include "widgets/widget_game.h"
#include "widgets/widget_board.h"
#include "widgets/widget_timer.h"
#include "widgets/widget_framerate.h"
#include "widgets/widget_countdown.h"

#include "console_screen.h"

#include <stdlib.h>

static struct Widget *s_widgets[WidgetId_Count] = {};


// Array with the list of ID for the priority frame / priority input


static void on_mouse_mouvement_callback( screenpos const oldPos, screenpos const newPos )
{
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget && widget->callbacks.mouseMoveCb )
        {
            widget->callbacks.mouseMoveCb( widget, oldPos, newPos );
        }
    }
}


static void on_screen_resize_callback( vec2u16 oldSize, vec2u16 newSize )
{
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( !widget ) continue;

		widget_utils_calculate_truncation( &widget->box, newSize );

        // TODO If the size doesn't change anything on the position/truncation of the widget, don't redraw for nothing.

        // TODO For each widget, calculate if the widget is truncated or not.
        // TODO If the widget go from not truncated to truncated, clear the widget view.

        if ( widget->box.borderOption != WidgetBorderOption_INVISIBLE ) // Ou truncated and display on truncate
        {
            widget_utils_draw_borders( &widget->box );
        }

        // TODO If not anymore truncated, call the redraw callback

/*        if ( widget->callbacks.clearCb )
        {
            widget->callbacks.clearCb( widget );
        }*/
    }
}


bool widget_global_init( void )
{
    s_widgets[WidgetId_GAME] = widget_game_create();
    s_widgets[WidgetId_FRAMERATE] = widget_framerate_create();
	s_widgets[WidgetId_BOARD] = widget_board_create();
    s_widgets[WidgetId_TIMER] = widget_timer_create();
	// boardSummary
    s_widgets[WidgetId_COUNTDOWN] = widget_countdown_create();
    // Init others widgets [...]

    // Register the widgets on event based updates (mouse, keyboard, resize, ...)
    mouse_register_on_mouse_mouvement_callback( on_mouse_mouvement_callback );
    console_screen_register_on_resize_callback( on_screen_resize_callback );
    // TODO add keyboard input
    return true;
}


void widget_global_uninit( void )
{
    for ( enum WidgetId idx = 0; idx < WidgetId_Count; ++idx )
    {
        if ( s_widgets[idx] )
        {
            free( s_widgets[idx] );
        }
    }
}


bool widget_exists( enum WidgetId const id )
{
    return s_widgets[id] != NULL;
}


struct Widget *widget_optget( enum WidgetId id )
{
    return s_widgets[id];
}


// /////////////////////////////////////////////////////////


void widget_frame( void )
{
    // Need to define an order of frame, because some popups can be put on top of the others
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget && widget->callbacks.frameCb )
        {
            widget->callbacks.frameCb( widget );
        }
    }
}
