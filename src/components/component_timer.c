#include "components/component_timer.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "mastermind.h"
#include "game.h"

#include "time_units.h"
#include "rect.h"

#include <stdlib.h>
#include <string.h>


enum TimerStatus
{
    TimerStatus_NOT_STARTED,
    TimerStatus_RUNNING,
    TimerStatus_PAUSED,
};


struct ComponentTimer
{
    struct ComponentHeader header;

    enum TimerStatus status;
    nsecond totalDuration;
    nsecond lastUpdateTimestamp;
    struct Rect rect;
    screenpos dispPos;
};
#define CAST_TO_COMP( _header ) ( ( struct ComponentTimer * )( _header ) )


bool widget_timer_start( struct ComponentHeader *header )
{
    assert( header->id == ComponentId_TIMER );
    struct ComponentTimer *comp = (struct ComponentTimer *)header;

    if ( comp->status != TimerStatus_NOT_STARTED ) return false;

    comp->lastUpdateTimestamp = time_get_timestamp_nsec();
    comp->totalDuration = 0;
    comp->status = TimerStatus_RUNNING;
    return true;
}


bool widget_timer_reset( struct ComponentHeader *const header )
{
    assert( header->id == ComponentId_TIMER );
    struct ComponentTimer *const comp = (struct ComponentTimer *const)header;

    if ( comp->status != TimerStatus_RUNNING && comp->status != TimerStatus_PAUSED ) return false;

    comp->totalDuration = 0;
    comp->status = TimerStatus_NOT_STARTED;
    return false;
}


bool widget_timer_pause( struct ComponentHeader *const header )
{
    assert( header->id == ComponentId_TIMER );
    struct ComponentTimer *const comp = (struct ComponentTimer *const)header;

    if ( comp->status != TimerStatus_RUNNING ) return false;

    comp->status = TimerStatus_PAUSED;

    return true;
}


bool widget_timer_resume( struct ComponentHeader *const header )
{
    assert( header->id == ComponentId_TIMER );
    struct ComponentTimer *const comp = (struct ComponentTimer *const)header;

    if ( comp->status != TimerStatus_PAUSED ) return false;

    comp->lastUpdateTimestamp = time_get_timestamp_nsec();
    comp->status = TimerStatus_RUNNING;

    return true;
}


static void write_timer_update( struct ComponentTimer const *comp )
{
    second const totalDuration = time_nsec_to_sec( comp->totalDuration );
    hour const hours     = ( totalDuration / 3600 );
    minute const minutes = ( totalDuration % 3600 ) / 60;
    second const seconds = totalDuration % 60;

    style_update( STYLE( FGColor_WHITE ) );
    cursor_update_pos( comp->dispPos );
    term_write( L"%02u:%02u:%02u", hours, minutes, seconds );
}


void frame_callback( struct ComponentHeader *header )
{
    struct ComponentTimer *comp = CAST_TO_COMP( header );

    if ( comp->status != TimerStatus_RUNNING )
        return;

    second const oldDuration = time_nsec_to_sec( comp->totalDuration );
    nsecond const newTimestamp = time_get_timestamp_nsec();
    nsecond const elapsedTime = newTimestamp - comp->lastUpdateTimestamp;

    comp->totalDuration += elapsedTime;
    comp->lastUpdateTimestamp = newTimestamp;

    second const newDuration = time_nsec_to_sec( comp->totalDuration );
    if ( oldDuration != newDuration )
    {
        write_timer_update( comp );
    }
}


static void event_received_callback( struct ComponentHeader *header, enum EventType event, struct EventData const *data )
{
	if ( event == EventType_NEW_GAME )
	{
        widget_timer_reset( header );
        widget_timer_start( header );
        write_timer_update( CAST_TO_COMP( header ) );
	}
	else if ( event == EventType_GAME_LOST || event == EventType_GAME_WON )
	{
        widget_timer_pause( header );
	}
}


static void enable_callback( struct ComponentHeader *header )
{
    struct ComponentTimer *comp = CAST_TO_COMP( header );
    rect_draw_borders( &CAST_TO_COMP( header )->rect, L"Timer" );
    write_timer_update( CAST_TO_COMP( header ) );
}


static void disable_callback( struct ComponentHeader *header )
{
    struct ComponentTimer *comp = CAST_TO_COMP( header );
    rect_clear( &comp->rect );
}


struct ComponentHeader *component_timer_create( void )
{
    struct ComponentTimer *const comp = calloc( 1, sizeof( struct ComponentTimer ) );
    if ( !comp ) return NULL;

	component_make_header( &comp->header, ComponentId_TIMER, false );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->enableCb = enable_callback;
    callbacks->disableCb = disable_callback;
    callbacks->frameCb = frame_callback;
    callbacks->eventReceivedCb = event_received_callback;

    screenpos const boxUL = SCREENPOS( 95, 2 );
    comp->rect = rect_make( boxUL, VEC2U16( 25, 3 ) );
    comp->dispPos = SCREENPOS( boxUL.x + 8, boxUL.y + 1 );

    return CAST_TO_COMP_HEADER( comp );
}
