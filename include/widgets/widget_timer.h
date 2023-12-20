#pragma once

#include "core_types.h"
#include "core_unions.h"
#include "time_units.h"

#include "widgets/widgets.h"
#include "widgets/widget_definition.h"

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

void widget_timer_start( struct WidgetTimer *timer );
void widget_timer_pause( struct WidgetTimer *timer );
void widget_timer_reset( struct WidgetTimer *timer );
void widget_timer_frame( struct WidgetTimer *timer );
void widget_timer_set_duration( struct WidgetTimer *timer, seconds duration  );

// redraw -> borders + content
void widget_timer_redraw( struct WidgetTimer *timer );
// update -> redraw content only
void widget_timer_update( struct WidgetTimer *timer, bool forceUpdate );

void widget_draw_borders( struct WidgetScreenData *screenData );

struct WidgetTimer *widget_timer_get_instance( void );
