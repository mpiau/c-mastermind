#pragma once

#include "core_types.h"
#include "time_units.h"

struct ComponentHeader *widget_countdown_create( void );

void widget_countdown_start( struct ComponentHeader *widget );
void widget_countdown_pause( struct ComponentHeader *widget );
void widget_countdown_resume( struct ComponentHeader *widget );

void widget_countdown_reset( struct ComponentHeader *widget );
void widget_countdown_set_duration( struct ComponentHeader *widget, second duration );
