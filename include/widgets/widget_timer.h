#pragma once

#include "core_types.h"
#include "core_unions.h"
#include "time_units.h"

enum WidgetType
{
    WidgetType_TIMER,
    WidgetType_BOARD_SUMMARY
};


struct WidgetScreenData
{
    screenpos upLeft;
    screenpos bottomRight;
    utf16     *name;
};

// Il faudrait un start() / stop() pour jouer avec la pause (ajoute a lastTimestamp la diff de temps avec la pause )
// play with elapsed time et garder tout en nano ? ça rajoute des / 1000[...]00 though
// Il faut que remaining == total duration tant que pas start

// should have the same width as WidgetBoardSummary, how could we make the dependency more explicit ?
struct WidgetTimer // Not Timer, countdown !
{
    enum WidgetType type;
    struct WidgetScreenData screenData;
    // Specific data to timer
    seconds endTimerTimestamp;
    seconds lastUpdateTimestamp;
    seconds totalDuration;
    bool finished;
};

void widget_timer_create( );

// redraw -> borders + content
void widget_timer_redraw( struct WidgetTimer *timer );
// update -> redraw content only
void widget_timer_update( struct WidgetTimer *timer, bool forceUpdate );

void widget_draw_borders( struct WidgetScreenData *screenData );
