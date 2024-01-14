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
    struct Rect box;
    screenpos dispPos;
};


bool widget_timer_start( struct ComponentHeader *widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct ComponentTimer *timer = (struct ComponentTimer *)widget;

    if ( timer->status != TimerStatus_NOT_STARTED ) return false;

    timer->lastUpdateTimestamp = time_get_timestamp_nsec();
    timer->totalDuration = 0;
    timer->status = TimerStatus_RUNNING;
    return true;
}


bool widget_timer_reset( struct ComponentHeader *const widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct ComponentTimer *const timer = (struct ComponentTimer *const)widget;

    if ( timer->status != TimerStatus_RUNNING && timer->status != TimerStatus_PAUSED ) return false;

    timer->totalDuration = 0;
    timer->status = TimerStatus_NOT_STARTED;
    return false;
}


bool widget_timer_pause( struct ComponentHeader *const widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct ComponentTimer *const timer = (struct ComponentTimer *const)widget;

    if ( timer->status != TimerStatus_RUNNING ) return false;

    timer->status = TimerStatus_PAUSED;

    return true;
}


bool widget_timer_resume( struct ComponentHeader *const widget )
{
    assert( widget->id == ComponentId_TIMER );
    struct ComponentTimer *const timer = (struct ComponentTimer *const)widget;

    if ( timer->status != TimerStatus_PAUSED ) return false;

    timer->lastUpdateTimestamp = time_get_timestamp_nsec();
    timer->status = TimerStatus_RUNNING;

    return true;
}


static void on_refresh_callback( struct ComponentHeader const *header )
{
    struct ComponentTimer const *timer = (struct ComponentTimer const *)header;

    rect_draw_borders( &timer->box, L"Timer" );

    second const totalDuration = time_nsec_to_sec( timer->totalDuration );
    hour const hours  = ( totalDuration / 3600 );
    minute const minutes = ( totalDuration % 3600 ) / 60;
    second const seconds = totalDuration % 60;
    screenpos ul = rect_get_ul_corner( &timer->box );

    style_update( STYLE( FGColor_WHITE ) );
    cursor_update_yx( timer->dispPos.y, timer->dispPos.x  );
    term_write( L"%02u:%02u:%02u", hours, minutes, seconds );
}


void frame_callback( struct ComponentHeader *widget )
{
    struct ComponentTimer *timer = (struct ComponentTimer *)widget;

    if ( timer->status != TimerStatus_RUNNING )
        return;

    second const oldDuration = time_nsec_to_sec( timer->totalDuration );
    nsecond const newTimestamp = time_get_timestamp_nsec();
    nsecond const elapsedTime = newTimestamp - timer->lastUpdateTimestamp;

    timer->totalDuration += elapsedTime;
    timer->lastUpdateTimestamp = newTimestamp;

    second const newDuration = time_nsec_to_sec( timer->totalDuration );
    if ( oldDuration != newDuration )
    {
		widget->refreshNeeded = true;
    }
}


static void on_game_update_callback( struct ComponentHeader *header, enum GameUpdateType type )
{
	if ( type == GameUpdateType_GAME_NEW )
	{
        usize const nbTurns = mastermind_get_total_turns();
        usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();

        vec2u16 const boxSize = (vec2u16) {
            .x = 4 /*borders + space each side*/ + ( nbPiecesPerTurn * 2 ) - 1 /*pegs*/ + 4 /*turn display*/ + nbPiecesPerTurn /*pins*/,
            .y = 3
        };
    	// We want to keep the board on the right of the screen, whether we have 4 or 6 pegs to display.
	    screenpos const boxUL = SCREENPOS( GAME_SIZE_WIDTH - boxSize.w, 3 );
        widget_timer_reset( header );
        widget_timer_start( header );
        struct ComponentTimer *timer = (struct ComponentTimer *)header;
        timer->box = rect_make( boxUL, boxSize );
        timer->dispPos = SCREENPOS( boxUL.x + ( ( boxSize.w - 8 ) / 2 ), boxUL.y + 1 );
		header->refreshNeeded = true;
		header->enabled = true;
	}
	else if ( type == GameUpdateType_GAME_FINISHED )
	{
        widget_timer_pause( header );
		header->refreshNeeded = true;
	}
}


struct ComponentHeader *component_timer_create( void )
{
    struct ComponentTimer *const comp = calloc( 1, sizeof( struct ComponentTimer ) );
    if ( !comp ) return NULL;

	component_make_header( &comp->header, ComponentId_TIMER, false );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->frameCb = frame_callback;
    callbacks->refreshCb = on_refresh_callback;
    callbacks->gameUpdateCb = on_game_update_callback;

    return (struct ComponentHeader *)comp;
}
