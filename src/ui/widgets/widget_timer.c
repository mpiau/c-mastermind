#include "ui/widgets.h"
#include "rect.h"
#include "time_units.h"
#include "terminal/terminal.h"
#include "events.h"

#include <stdlib.h>

enum TimerStatus
{
    TimerStatus_NOT_STARTED,
    TimerStatus_RUNNING,
    TimerStatus_PAUSED,
};


struct WidgetTimer
{
    struct Widget base;

    enum TimerStatus status;
    nsecond totalDuration;
    nsecond lastUpdateTimestamp;
    struct Rect box;
    screenpos dispPos;
    struct Style style;
};


static void draw_update( struct WidgetTimer const *widget )
{
    second const totalDuration = time_nsec_to_sec( widget->totalDuration );
    hour const hours     = ( totalDuration / 3600 );
    minute const minutes = ( totalDuration % 3600 ) / 60;
    second const seconds = totalDuration % 60;

    style_update( widget->style );
    cursor_update_pos( widget->dispPos );
    term_write( L"%02u:%02u:%02u", hours, minutes, seconds );
}


static void start_timer( struct WidgetTimer *widget )
{
    if ( widget->status == TimerStatus_NOT_STARTED )
    {
        widget->lastUpdateTimestamp = time_get_timestamp_nsec();
        widget->totalDuration = 0;
        widget->status = TimerStatus_RUNNING;
    }
}


static void reset_timer( struct WidgetTimer *widget )
{
    if ( widget->status == TimerStatus_RUNNING || widget->status == TimerStatus_PAUSED )
    {
        widget->totalDuration = 0;
        widget->status = TimerStatus_NOT_STARTED;
    }
}


static void pause_timer( struct WidgetTimer *widget )
{
    // NOTE:
    // Do we want to change the style of the timer when paused ? (blinking, ...)

    if ( widget->status == TimerStatus_RUNNING )
    {
        widget->status = TimerStatus_PAUSED;
    }
}

// Not used (yet).
/*static void resume_timer( struct WidgetTimer *widget )
{
    if ( widget->status == TimerStatus_PAUSED )
    {
        widget->lastUpdateTimestamp = time_get_timestamp_nsec();
        widget->status = TimerStatus_RUNNING;
    }
}*/


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetTimer *widget = (struct WidgetTimer *)subscriber;

   	if ( event->type == EventType_GAME_NEW )
	{
        reset_timer( widget );
        start_timer( widget );
        widget->style = STYLE( FGColor_WHITE );
        draw_update( widget );
	}
	else if ( event->type == EventType_GAME_LOST || event->type == EventType_GAME_WON )
	{
        pause_timer( widget );
        widget->style = STYLE_WITH_ATTR( FGColor_WHITE, Attr_FAINT );
        draw_update( widget );
	}
    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
    struct WidgetTimer *widget = (struct WidgetTimer *)base;
    rect_draw_borders( &widget->box, L"Timer" );
    draw_update( widget );
}


static void disable_callback( struct Widget *base )
{
    struct WidgetTimer *widget = (struct WidgetTimer *)base;
    rect_clear( &widget->box );
}


static void frame_callback( struct Widget *base )
{
    struct WidgetTimer *widget = (struct WidgetTimer *)base;

    if ( widget->status != TimerStatus_RUNNING )
        return;

    second const oldDuration = time_nsec_to_sec( widget->totalDuration );
    nsecond const newTimestamp = time_get_timestamp_nsec();
    nsecond const elapsedTime = newTimestamp - widget->lastUpdateTimestamp;

    widget->totalDuration += elapsedTime;
    widget->lastUpdateTimestamp = newTimestamp;

    second const newDuration = time_nsec_to_sec( widget->totalDuration );
    if ( oldDuration != newDuration )
    {
        draw_update( widget );
    }
}


struct Widget *widget_timer_create( void )
{
    struct WidgetTimer *const widget = calloc( 1, sizeof( struct WidgetTimer ) );
    if ( !widget ) return NULL;

    widget->base.name = "Timer";
    widget->base.enabledScenes = UIScene_IN_GAME;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;
    widget->base.frameCb = frame_callback;

    // Widget specific    
    screenpos const boxUL = SCREENPOS( 95, 2 );
    widget->box = rect_make( boxUL, VEC2U16( 25, 3 ) );
    widget->dispPos = SCREENPOS( boxUL.x + 8, boxUL.y + 1 );
    widget->style = STYLE( FGColor_WHITE );

    event_register( widget, on_event_callback );
    event_subscribe( widget, EventType_GAME_NEW | EventType_GAME_LOST | EventType_GAME_WON );

    return (struct Widget *)widget;
}
