/*#include "components/component_countdown.h"

#include "time_units.h"
#include "components/component_header.h"
#include "terminal/terminal.h"
#include "rect.h"

#include <stdlib.h>
#include <string.h>
*/
/*

enum CountdownStatus
{
	CountdownStatus_NOT_STARTED,
	CountdownStatus_PAUSED,
	CountdownStatus_IN_PROGRESS,
	CountdownStatus_TIME_OUT
};

struct WidgetCountdown
{
	struct ComponentHeader header;

    // Specific data to timer
	enum CountdownStatus status;
    nsecond endTimerTimestamp;
    nsecond lastUpdateTimestamp;
    nsecond totalDuration;
    bool finished;
    struct Rect box;
};


static void on_refresh_callback( struct ComponentHeader const *widget )
{
    struct WidgetCountdown const *countdown = (struct WidgetCountdown const *)widget;

    cursor_update_pos( rect_get_ul_corner( &countdown->box ) );

    u32 const width = countdown->box.size.w;

    if ( countdown->status == CountdownStatus_IN_PROGRESS && countdown->lastUpdateTimestamp > countdown->endTimerTimestamp )
    {
        utf16 msg[] = L"TIME OUT";
        style_update( STYLE_WITH_ATTR( FGColor_RED, Attr_BLINK ) );
        cursor_move_right_by( ( width - ARR_COUNT( msg ) ) / 2 ); // center
        term_write( L"%S", msg );
        //countdown->status = CountdownStatus_TIME_OUT;
        return;
    }

    nsecond const remainingTimeNsec = countdown->status == CountdownStatus_NOT_STARTED
		? countdown->totalDuration
		: countdown->endTimerTimestamp - countdown->lastUpdateTimestamp;

	second const remainingTimeSec = time_nsec_to_sec( remainingTimeNsec );

    hour const hours  = remainingTimeSec / 3600;
    minute const minutes = ( remainingTimeSec % 3600 ) / 60;
    second const seconds = remainingTimeSec % 60;

    // Blue, yellow, blinking yellow, red, blinking red ?
    if ( remainingTimeNsec < ( countdown->totalDuration * 10 ) / 100 )
    {
        style_update( STYLE_WITH_ATTR( FGColor_RED, Attr_BLINK ) );
    }
    else if ( remainingTimeNsec < ( countdown->totalDuration * 25 ) / 100 )
    {
        style_update( STYLE( FGColor_RED ) );
    }
    else if ( remainingTimeNsec < ( countdown->totalDuration * 60 ) / 100 )
    {
        style_update( STYLE( FGColor_YELLOW ) );
    }
    else
    {
        style_update( STYLE( FGColor_CYAN ) );
    }

    cursor_move_right_by( ( width - 8 ) / 2 ); // center
    term_write( L"%02u:%02u:%02u", hours, minutes, seconds );
    term_write( L"\033[0m" ); // blink mode off in case
}


static void frame_callback( struct ComponentHeader *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

    nsecond const newTimestamp = time_get_timestamp_nsec();
	// Would be better to check if the difference between the 2 is greater than 1 second no ?
	bool const equalsInSeconds = time_nsec_to_sec( newTimestamp ) == time_nsec_to_sec( countdown->lastUpdateTimestamp );
    if ( countdown->status == CountdownStatus_TIME_OUT || equalsInSeconds ) return;

	if ( countdown->status != CountdownStatus_PAUSED )
	{
	    countdown->lastUpdateTimestamp = newTimestamp;
		on_refresh_callback( widget );
	}
}


struct ComponentHeader *component_countdown_create( void )
{
    struct WidgetCountdown *const countdown = malloc( sizeof( struct WidgetCountdown ) );
    if ( !countdown ) return NULL;
    memset( countdown, 0, sizeof( *countdown ) );

	struct ComponentHeader *const widget = &countdown->header;

    widget->id = ComponentId_COUNTDOWN;
	widget->enabled = false;

    countdown->box = rect_make( (screenpos) { .x = 1, .y = 1 }, (vec2u16) { .x = 16, .y = 1 } );

    countdown->header.callbacks.frameCb = frame_callback;
	countdown->header.callbacks.refreshCb = on_refresh_callback;

	// Widget specific

	countdown->status = CountdownStatus_NOT_STARTED;
	countdown->totalDuration = time_sec_to_nsec( 60 );

    return (struct ComponentHeader *)countdown;
}


// //////////////////////////////////////////////////////////////////


void widget_countdown_start( struct ComponentHeader *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	if ( countdown->status != CountdownStatus_NOT_STARTED ) return;
	assert( countdown->totalDuration > 0 );

	countdown->lastUpdateTimestamp = time_get_timestamp_nsec();
	countdown->endTimerTimestamp = countdown->lastUpdateTimestamp + countdown->totalDuration;
	countdown->status = CountdownStatus_IN_PROGRESS;
}


void widget_countdown_pause( struct ComponentHeader *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	countdown->status = CountdownStatus_PAUSED;
}


void widget_countdown_resume( struct ComponentHeader *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	nsecond const timestamp = time_get_timestamp_nsec();
	countdown->endTimerTimestamp += ( timestamp - countdown->lastUpdateTimestamp );
	countdown->lastUpdateTimestamp = timestamp;
	countdown->status = CountdownStatus_IN_PROGRESS;
}


void widget_countdown_reset( struct ComponentHeader *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	countdown->status = CountdownStatus_NOT_STARTED;
}


void widget_countdown_set_duration( struct ComponentHeader *widget, second const duration )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	if ( duration == 0 ) return;

	countdown->totalDuration = time_sec_to_nsec( duration );
}

*/