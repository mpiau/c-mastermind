#pragma once

#include "core_types.h"

// Forward declaration
struct Widget;

struct Widget *widget_timer_create( void );

bool widget_timer_start( struct Widget *widget );
