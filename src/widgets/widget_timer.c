#include "widgets/widget_timer.h"

#include "widgets/widget_definition.h"

#include "console_screen.h"
#include "fps_counter.h" // Not making sense, but need the current timestamp. FIX IT

struct WidgetTimer
{
	struct Widget header;

    seconds startTimestamp;
    seconds lastUpdateTimestamp;
};

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

void frame_callback( struct Widget *widget )
{
	assert( widget->id == WidgetId_TIMER );
    struct WidgetTimer *timer = (struct WidgetTimer *)widget;

    seconds newTimestamp = get_timestamp_nanoseconds() / NANOSECONDS;
	if ( newTimestamp == timer->lastUpdateTimestamp ) return;

	screenpos const upLeft = timer->header.border.upLeft;
    console_cursor_set_position( upLeft.y + 1, upLeft.x + 1 );

    u32 const width = timer->header.border.size.x;
    u32 const widthNoBorders = width - 2;

    seconds const elapsedTime = newTimestamp - timer->startTimestamp;

    hours   const hours = elapsedTime / 3600;
    minutes const minutes = ( elapsedTime % 3600 ) / 60;
    seconds const seconds = elapsedTime % 60;

    console_color_fg( ConsoleColorFG_CYAN );
    console_cursor_move_right_by( ( widthNoBorders - 8 ) / 2 ); // center
    console_draw( L"%02u:%02u:%02u", hours, minutes, seconds );
	console_color_reset();

    timer->lastUpdateTimestamp = newTimestamp;
}


struct Widget *widget_timer_create( void )
{
	struct WidgetTimer *const timer = malloc( sizeof( struct WidgetTimer ) );
    if ( !timer ) return NULL;

	vec2u16 const screenSize = console_screen_get_size();
	timer->header.id = WidgetId_TIMER;
	timer->header.border.upLeft = (screenpos) { .x = screenSize.x - 19, .y = 18 };
	timer->header.border.size = (vec2u16) { .x = 18, .y = 3 };
	timer->header.border.optTitle = L"Timer";

	timer->header.callbacks.frameCb = frame_callback;

	timer->startTimestamp = get_timestamp_nanoseconds() / NANOSECONDS;
	timer->lastUpdateTimestamp = timer->startTimestamp;

	return (struct Widget *)timer;
}
