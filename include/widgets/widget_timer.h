#pragma once

#include "core_types.h"

// Forward declaration
struct ComponentHeader;

struct ComponentHeader *widget_timer_create( void );

bool widget_timer_start( struct ComponentHeader *widget );
