#include "widgets/widget_countdown.h"

#include "fps_counter.h" // should be only time_units
#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

enum CountdownStatus
{
	CountdownStatus_NOT_STARTED,
	CountdownStatus_PAUSED,
	CountdownStatus_IN_PROGRESS,
	CountdownStatus_TIME_OUT
};

struct WidgetCountdown
{
	struct Widget header;

    // Specific data to timer
	enum CountdownStatus status;
    seconds endTimerTimestamp;
    seconds lastUpdateTimestamp;
    seconds totalDuration;
    bool finished;
};


static void frame_callback( struct Widget *widget )
{
	assert( widget->id == WidgetId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

    seconds newTimestamp = get_timestamp_nanoseconds() / NANOSECONDS;
    if ( countdown->status == CountdownStatus_TIME_OUT || newTimestamp == countdown->lastUpdateTimestamp ) return;

    console_cursor_set_position( countdown->header.border.upLeft.y + 1, countdown->header.border.upLeft.x + 1 );

    u32 const width = countdown->header.border.size.x;
    u32 const widthNoBorders = width - 2;

    if ( countdown->status == CountdownStatus_IN_PROGRESS && newTimestamp > countdown->endTimerTimestamp )
    {
        utf16 msg[] = L"TIME OUT";
        console_color_fg( ConsoleColorFG_RED ); 
        console_draw( L"\033[5m" ); // blink mode on
        console_cursor_move_right_by( ( width - ARR_COUNT( msg ) ) / 2 ); // center
        console_draw( L"%S", msg );
        console_draw( L"\033[0m" ); // blink mode off
        countdown->status = CountdownStatus_TIME_OUT;
        return;
    }

    seconds const remainingTime = countdown->status == CountdownStatus_NOT_STARTED
		? countdown->totalDuration
		: countdown->status == CountdownStatus_PAUSED
			? countdown->endTimerTimestamp - countdown->lastUpdateTimestamp
			: countdown->endTimerTimestamp - newTimestamp;

    hours     const hours = remainingTime / 3600;
    minutes const minutes = ( remainingTime % 3600 ) / 60;
    seconds const seconds = remainingTime % 60;

    // Blue, yellow, blinking yellow, red, blinking red ?
    if ( remainingTime < ( countdown->totalDuration * 10 ) / 100 )
    {
        console_color_fg( ConsoleColorFG_RED ); 
        console_draw( L"\033[5m" ); // blink mode on
    }
    else if ( remainingTime < ( countdown->totalDuration * 25 ) / 100 )
    {
        console_color_fg( ConsoleColorFG_RED ); 
    }
    else if ( remainingTime < ( countdown->totalDuration * 60 ) / 100 /* <40% */ )
    {
        console_color_fg( ConsoleColorFG_YELLOW );
    }
    else
    {
        console_color_fg( ConsoleColorFG_CYAN );
    }

    console_cursor_move_right_by( ( widthNoBorders - 8 ) / 2 ); // center
    console_draw( L"%02u:%02u:%02u", hours, minutes, seconds );
    console_draw( L"\033[0m" ); // blink mode off in case

	if ( countdown->status != CountdownStatus_PAUSED )
	{
	    countdown->lastUpdateTimestamp = newTimestamp;
	}
}


struct Widget *widget_countdown_create( void )
{
    struct WidgetCountdown *const countdown = malloc( sizeof( struct WidgetCountdown ) );
    if ( !countdown ) return NULL;

    countdown->header.id = WidgetId_COUNTDOWN;

    countdown->header.callbacks.frameCb = frame_callback;

	countdown->header.border.upLeft = (vec2u16) { .x = 40, .y = 18 };
	countdown->header.border.size   = (vec2u16) { .x = 12, .y = 3 };

	countdown->status = CountdownStatus_NOT_STARTED;
	countdown->totalDuration = 60;

    return (struct Widget *)countdown;
}


// //////////////////////////////////////////////////////////////////


void widget_countdown_start( struct Widget *widget )
{
	assert( widget->id == WidgetId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	if ( countdown->status != CountdownStatus_NOT_STARTED ) return;
	assert( countdown->totalDuration > 0 );

	countdown->lastUpdateTimestamp = get_timestamp_nanoseconds() / NANOSECONDS;
	countdown->endTimerTimestamp = countdown->lastUpdateTimestamp + countdown->totalDuration;
	countdown->status = CountdownStatus_IN_PROGRESS;
}


void widget_countdown_pause( struct Widget *widget )
{
	assert( widget->id == WidgetId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	countdown->status = CountdownStatus_PAUSED;
}


void widget_countdown_resume( struct Widget *widget )
{
	assert( widget->id == WidgetId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	nanoseconds timestamp = get_timestamp_nanoseconds() / NANOSECONDS;

	countdown->endTimerTimestamp += ( timestamp - countdown->lastUpdateTimestamp );
	countdown->lastUpdateTimestamp = timestamp;
	countdown->status = CountdownStatus_IN_PROGRESS;
}


void widget_countdown_reset( struct Widget *widget )
{
	assert( widget->id == WidgetId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	countdown->status = CountdownStatus_NOT_STARTED;
}


void widget_countdown_set_duration( struct Widget *widget, seconds duration )
{
	assert( widget->id == WidgetId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	if ( duration == 0 ) return;

	countdown->totalDuration = duration;
}
