#include "widgets/widget_countdown.h"

#include "time_units.h"
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
    nsec endTimerTimestamp;
    nsec lastUpdateTimestamp;
    nsec totalDuration;
    bool finished;
};


static void redraw_callback( struct Widget *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
	if ( !widget->enabled ) return;
	if ( widget->box.truncatedStatus != WidgetTruncatedStatus_NONE ) return;

    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

    console_cursor_set_position( countdown->header.box.contentUpLeft.y, countdown->header.box.contentUpLeft.x );

    u32 const width = countdown->header.box.contentBottomRight.x - countdown->header.box.contentUpLeft.x + 1;

    if ( countdown->status == CountdownStatus_IN_PROGRESS && countdown->lastUpdateTimestamp > countdown->endTimerTimestamp )
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

    nsec const remainingTimeNsec = countdown->status == CountdownStatus_NOT_STARTED
		? countdown->totalDuration
		: countdown->endTimerTimestamp - countdown->lastUpdateTimestamp;

	sec const remainingTimeSec = time_nsec_to_sec( remainingTimeNsec );

    hour const hours  = remainingTimeSec / 3600;
    min const minutes = ( remainingTimeSec % 3600 ) / 60;
    sec const seconds = remainingTimeSec % 60;

    // Blue, yellow, blinking yellow, red, blinking red ?
    if ( remainingTimeNsec < ( countdown->totalDuration * 10 ) / 100 )
    {
        console_color_fg( ConsoleColorFG_RED ); 
        console_draw( L"\033[5m" ); // blink mode on
    }
    else if ( remainingTimeNsec < ( countdown->totalDuration * 25 ) / 100 )
    {
        console_color_fg( ConsoleColorFG_RED ); 
    }
    else if ( remainingTimeNsec < ( countdown->totalDuration * 60 ) / 100 /* <40% */ )
    {
        console_color_fg( ConsoleColorFG_YELLOW );
    }
    else
    {
        console_color_fg( ConsoleColorFG_CYAN );
    }

    console_cursor_move_right_by( ( width - 8 ) / 2 ); // center
    console_draw( L"%02u:%02u:%02u", hours, minutes, seconds );
    console_draw( L"\033[0m" ); // blink mode off in case
}


static void frame_callback( struct Widget *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

    nsec const newTimestamp = time_get_timestamp_nsec();
	// Would be better to check if the difference between the 2 is greater than 1 second no ?
	bool const equalsInSeconds = time_nsec_to_sec( newTimestamp ) == time_nsec_to_sec( countdown->lastUpdateTimestamp );
    if ( countdown->status == CountdownStatus_TIME_OUT || equalsInSeconds ) return;

	if ( countdown->status != CountdownStatus_PAUSED )
	{
	    countdown->lastUpdateTimestamp = newTimestamp;
		redraw_callback( widget );
	}
}


struct Widget *widget_countdown_create( void )
{
    struct WidgetCountdown *const countdown = malloc( sizeof( struct WidgetCountdown ) );
    if ( !countdown ) return NULL;
    memset( countdown, 0, sizeof( *countdown ) );

	struct Widget *const widget = &countdown->header;

    widget->id = ComponentId_COUNTDOWN;
	widget->enabled = true;

	assert( widget_exists( ComponentId_TIMER ) );
	struct WidgetBox const *TimerBox = &widget_optget( ComponentId_TIMER )->box;

    screenpos const borderUpLeft = (screenpos) { .x = TimerBox->borderUpLeft.x, .y = TimerBox->borderBottomRight.y + 1 };
    vec2u16 const contentSize  = (vec2u16)   { .x = 16, .y = 1 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;
	widget_utils_set_title( &widget->box, L"Countdown", ConsoleColorFG_CYAN );

    countdown->header.callbacks.frameCb = frame_callback;
	countdown->header.callbacks.redrawCb = redraw_callback;
    countdown->header.callbacks.resizeCb = NULL;

	// Widget specific

	countdown->status = CountdownStatus_NOT_STARTED;
	countdown->totalDuration = time_sec_to_nsec( 60 );

    return (struct Widget *)countdown;
}


// //////////////////////////////////////////////////////////////////


void widget_countdown_start( struct Widget *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	if ( countdown->status != CountdownStatus_NOT_STARTED ) return;
	assert( countdown->totalDuration > 0 );

	countdown->lastUpdateTimestamp = time_get_timestamp_nsec();
	countdown->endTimerTimestamp = countdown->lastUpdateTimestamp + countdown->totalDuration;
	countdown->status = CountdownStatus_IN_PROGRESS;
}


void widget_countdown_pause( struct Widget *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	countdown->status = CountdownStatus_PAUSED;
}


void widget_countdown_resume( struct Widget *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	nsec const timestamp = time_get_timestamp_nsec();
	countdown->endTimerTimestamp += ( timestamp - countdown->lastUpdateTimestamp );
	countdown->lastUpdateTimestamp = timestamp;
	countdown->status = CountdownStatus_IN_PROGRESS;
}


void widget_countdown_reset( struct Widget *widget )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	countdown->status = CountdownStatus_NOT_STARTED;
}


void widget_countdown_set_duration( struct Widget *widget, sec const duration )
{
	assert( widget->id == ComponentId_COUNTDOWN );
    struct WidgetCountdown *countdown = (struct WidgetCountdown *)widget;

	if ( duration == 0 ) return;

	countdown->totalDuration = time_sec_to_nsec( duration );
}

