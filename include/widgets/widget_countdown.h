#pragma once

#include "core_types.h"
#include "time_units.h"

struct Widget *widget_countdown_create( void );

void widget_countdown_start( struct Widget *widget );
void widget_countdown_pause( struct Widget *widget );
void widget_countdown_resume( struct Widget *widget );

void widget_countdown_reset( struct Widget *widget );
void widget_countdown_set_duration( struct Widget *widget, seconds duration );
