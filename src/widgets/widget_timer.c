#include "widgets/widget_timer.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "console/console_screen.h"
#include "time_units.h"

enum TimerStatus
{
    TimerStatus_NOT_STARTED,
    TimerStatus_RUNNING,
    TimerStatus_PAUSED,
};


struct WidgetTimer
{
    struct Widget header;

    enum TimerStatus status;
    nsec totalDuration;
    nsec lastUpdateTimestamp;
};


static void redraw_callback( struct Widget *widget )
{
	return;
	
    assert( widget->id == ComponentId_TIMER );
    assert( widget->enabled );
    assert( widget->box.truncatedStatus == WidgetTruncatedStatus_NONE );

    struct WidgetTimer *timer = (struct WidgetTimer *)widget;

    // TODO If widget truncated, return. Don't display anything

    screenpos const contentUL = timer->header.box.contentUpLeft;
    screenpos const contentBR = timer->header.box.contentBottomRight;
    u32 const width = contentBR.x - contentUL.x + 1;

    sec const totalDuration = time_nsec_to_sec( timer->totalDuration );

    hour const hours  = ( totalDuration / 3600 );
    min const minutes = ( totalDuration % 3600 ) / 60;
    sec const seconds = totalDuration % 60;

    console_color_fg( ConsoleColorFG_WHITE );
    console_cursor_set_position( contentUL.y, contentUL.x + ( ( width - 8 ) / 2 ) );
    console_draw( L"%02u:%02u:%02u", hours, minutes, seconds );
    console_color_reset();
}


void frame_callback( struct Widget *widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct WidgetTimer *timer = (struct WidgetTimer *)widget;

    if ( timer->status != TimerStatus_RUNNING ) return;

    sec const oldDuration = time_nsec_to_sec( timer->totalDuration );
    nsec const newTimestamp = time_get_timestamp_nsec();
    nsec const elapsedTime = newTimestamp - timer->lastUpdateTimestamp;

    timer->totalDuration += elapsedTime;
    timer->lastUpdateTimestamp = newTimestamp;

    sec const newDuration = time_nsec_to_sec( timer->totalDuration );
    if ( oldDuration != newDuration )
    {
		widget->redrawNeeded = true;
    }
}


struct Widget *widget_timer_create( void )
{
    struct WidgetTimer *const timer = malloc( sizeof( struct WidgetTimer ) );
    if ( !timer ) return NULL;
    memset( timer, 0, sizeof( *timer ) );

	struct Widget *const widget = &timer->header;

    widget->id = ComponentId_TIMER;
	widget->enabled = true;

	assert( widget_exists( ComponentId_BOARD ) );
	struct WidgetBox const *boardBox = &widget_optget( ComponentId_BOARD )->box;

    screenpos const borderUpLeft = (screenpos) { .x = boardBox->borderBottomRight.x + 7, .y = boardBox->borderUpLeft.y };
    screenpos const contentSize  = (vec2u16)   { .x = 16, .y = 1 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;
	widget_utils_set_title( &widget->box, L"Timer", ConsoleColorFG_MAGENTA );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
    callbacks->frameCb = frame_callback;
    callbacks->redrawCb = redraw_callback;
    callbacks->resizeCb = NULL;

	// Specific to widget 

    timer->totalDuration = 0;
    timer->status = TimerStatus_NOT_STARTED;

    return (struct Widget *)timer;
}


bool widget_timer_start( struct Widget *const widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_NOT_STARTED ) return false;

    timer->lastUpdateTimestamp = time_get_timestamp_nsec();
    timer->status = TimerStatus_RUNNING;
    return true;
}


bool widget_timer_reset( struct Widget *const widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_RUNNING && timer->status != TimerStatus_PAUSED ) return false;

    timer->totalDuration = 0;
    timer->status = TimerStatus_NOT_STARTED;
    return false;
}


bool widget_timer_pause( struct Widget *const widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_RUNNING ) return false;

    timer->status = TimerStatus_PAUSED;

    return true;
}


bool widget_timer_resume( struct Widget *const widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_PAUSED ) return false;

    timer->lastUpdateTimestamp = time_get_timestamp_nsec();
    timer->status = TimerStatus_RUNNING;

    return true;
}
