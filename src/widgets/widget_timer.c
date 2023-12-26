#include "widgets/widget_timer.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "console_screen.h"
#include "fps_counter.h" // Not making sense, but need the current timestamp. FIX IT

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
    nanoseconds      totalDuration;
    nanoseconds      lastUpdateTimestamp;
};


static void clear_callback( struct Widget *widget )
{
	// widget_utils_clear_content( &widget->border );
}


static void redraw_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_TIMER );
	if ( widget->visibilityStatus == WidgetVisibilityStatus_HIDDEN ) return;
	if ( widget->box.truncatedStatus != WidgetTruncatedStatus_NONE ) return;

    struct WidgetTimer *timer = (struct WidgetTimer *)widget;

    // TODO If widget truncated, return. Don't display anything

    screenpos const contentUL = timer->header.box.contentUpLeft;
    screenpos const contentBR = timer->header.box.contentBottomRight;
    u32 const width = contentBR.x - contentUL.x + 1;

    seconds const totalDuration = timer->totalDuration / NANOSECONDS;

    hours   const hours   = ( totalDuration / 3600 );
    minutes const minutes = ( totalDuration % 3600 ) / 60;
    seconds const seconds = totalDuration % 60;

    console_color_fg( ConsoleColorFG_WHITE );
    console_cursor_set_position( contentUL.y, contentUL.x + ( ( width - 8 ) / 2 ) );
    console_draw( L"%02u:%02u:%02u", hours, minutes, seconds );
    console_color_reset();
}


void frame_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_TIMER );
    struct WidgetTimer *timer = (struct WidgetTimer *)widget;

    if ( timer->status != TimerStatus_RUNNING ) return;

    seconds const oldDuration = timer->totalDuration / NANOSECONDS;
    nanoseconds const newTimestamp = get_timestamp_nanoseconds();
    nanoseconds const elapsedTime = newTimestamp - timer->lastUpdateTimestamp;

    timer->totalDuration += elapsedTime;
    timer->lastUpdateTimestamp = newTimestamp;

    seconds const newDuration = timer->totalDuration / NANOSECONDS;

    if ( oldDuration != newDuration )
    {
        redraw_callback( widget );
    }
}


struct Widget *widget_timer_create( void )
{
    struct WidgetTimer *const timer = malloc( sizeof( struct WidgetTimer ) );
    if ( !timer ) return NULL;

	struct Widget *const widget = &timer->header;

    widget->id = WidgetId_TIMER;
	widget->visibilityStatus = WidgetVisibilityStatus_VISIBLE;

	assert( widget_exists( WidgetId_BOARD ) );
	struct WidgetBox const *boardBox = &widget_optget( WidgetId_BOARD )->box;

    screenpos const borderUpLeft = (screenpos) { .x = boardBox->borderBottomRight.x + 1, .y = boardBox->borderUpLeft.y };
    screenpos const contentSize  = (vec2u16)   { .x = 16, .y = 1 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;
	widget_utils_set_title( &widget->box, L"Timer", ConsoleColorFG_MAGENTA );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
    callbacks->frameCb = frame_callback;
    callbacks->redrawCb = redraw_callback;
    callbacks->clearCb = clear_callback;

	// Specific to widget 

    timer->totalDuration = 0;
    timer->status = TimerStatus_NOT_STARTED;

    return (struct Widget *)timer;
}


bool widget_timer_start( struct Widget *const widget )
{
    assert( widget->id == WidgetId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_NOT_STARTED ) return false;

    timer->lastUpdateTimestamp = get_timestamp_nanoseconds();
    timer->status = TimerStatus_RUNNING;
    return true;
}


bool widget_timer_reset( struct Widget *const widget )
{
    assert( widget->id == WidgetId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_RUNNING && timer->status != TimerStatus_PAUSED ) return false;

    timer->totalDuration = 0;
    timer->status = TimerStatus_NOT_STARTED;
    return false;
}


bool widget_timer_pause( struct Widget *const widget )
{
    assert( widget->id == WidgetId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_RUNNING ) return false;

    timer->status = TimerStatus_PAUSED;

    return true;
}


bool widget_timer_resume( struct Widget *const widget )
{
    assert( widget->id == WidgetId_TIMER );
    struct WidgetTimer *const timer = (struct WidgetTimer *const)widget;

    if ( timer->status != TimerStatus_PAUSED ) return false;

    timer->lastUpdateTimestamp = get_timestamp_nanoseconds();
    timer->status = TimerStatus_RUNNING;

    return true;
}



/*
// redraw -> borders + content
void widget_timer_redraw( struct WidgetTimer *timer )
{
    widget_draw_borders( &timer->screenData );
    // widget_timer_update( timer, true );
}

// update -> redraw content only if needed
void widget_timer_update( struct WidgetTimer *timer, bool forceUpdate )
{
    seconds newTimestamp = get_timestamp_nanoseconds() / NANOSECONDS;
    if ( timer->finished || newTimestamp == timer->lastUpdateTimestamp ) return;

    console_cursor_set_position( timer->screenData.upLeft.y + 1, timer->screenData.upLeft.x + 1 );

    u32 const width = timer->screenData.bottomRight.x - timer->screenData.upLeft.x;
    u32 const widthNoBorders = width - 2;

    if ( newTimestamp > timer->endTimerTimestamp )
    {
        utf16 msg[] = L"TIME OUT";
        console_color_fg( ConsoleColorFG_RED ); 
        console_draw( L"\033[5m" ); // blink mode on
        console_cursor_move_right_by( ( width - ARR_COUNT( msg ) ) / 2 ); // center
        console_draw( L"%S", msg );
        console_draw( L"\033[0m" ); // blink mode off
        timer->finished = true;
        return;
    }

    seconds const remainingTime = timer->endTimerTimestamp - newTimestamp;

    hours     const hours = remainingTime / 3600;
    minutes const minutes = ( remainingTime % 3600 ) / 60;
    seconds const seconds = remainingTime % 60;

    // Blue, yellow, blinking yellow, red, blinking red ?
    if ( remainingTime < ( timer->totalDuration * 10 ) / 100 )
    {
        console_color_fg( ConsoleColorFG_RED ); 
        console_draw( L"\033[5m" ); // blink mode on
    }
    else if ( remainingTime < ( timer->totalDuration * 25 ) / 100 )
    {
        console_color_fg( ConsoleColorFG_RED ); 
    }
    else if ( remainingTime < ( timer->totalDuration * 60 ) / 100 )
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

    timer->lastUpdateTimestamp = newTimestamp;
}


// FOR ALL WIDGETS


void widget_draw_up_border( screenpos upLeft, utf16 *name, u32 const widthNoBorders )
{
    console_cursor_set_position( upLeft.y, upLeft.x );

    console_color_fg( ConsoleColorFG_WHITE );
    console_draw( L"┌" );
    
    // Title
    int titleWidth = 0;
    if ( name != NULL )
    {
        console_color_fg( ConsoleColorFG_GREEN );
        titleWidth = console_draw( L" %S ", name );
        console_color_fg( ConsoleColorFG_WHITE );
    }

    for ( int i = 0; i < widthNoBorders - titleWidth; ++ i )
    {
        console_draw( L"─" );
    }
    console_draw( L"┐" );
}

void widget_draw_vert_borders( screenpos upLeft, u32 height, u32 const widthNoBorders )
{
    for ( int y = 0; y < height; ++y )
    {
        console_cursor_set_position( upLeft.y + y, upLeft.x );
        console_draw( L"│" );
        console_cursor_move_right_by( widthNoBorders );
        console_draw( L"│" );
    }
}


void widget_draw_bottom_border( screenpos upLeft, u32 const widthNoBorders )
{
    console_cursor_set_position( upLeft.y, upLeft.x );

    console_draw( L"└" );
    for ( int i = 0; i < widthNoBorders; ++ i )
    {
        console_draw( L"─" );
    }
    console_draw( L"┘" );
}


void widget_draw_borders( struct WidgetScreenData *screenData )
{
    // Ensure screen data is valid :
    // top left and bottom right are valid between each other
    // The title fit in the given width
    // height > 2 because otherwise its too small to have borders
    u32 const width = screenData->bottomRight.x - screenData->upLeft.x;
    u32 const widthNoBorders = width - 2;
    u32 const height = screenData->bottomRight.y - screenData->upLeft.y;

    screenpos currPos = screenData->upLeft;

    widget_draw_up_border( currPos, screenData->name, widthNoBorders );
    currPos.y += 1;
    widget_draw_vert_borders( currPos, height, widthNoBorders );

    currPos.y = screenData->upLeft.y + height;
    widget_draw_bottom_border( currPos, widthNoBorders );
}*/

