#pragma once

#include "core_types.h"
#include "core_unions.h"

struct WidgetBox;

bool widget_is_truncated( struct WidgetBox const *box );
bool widget_is_out_of_bounds( struct WidgetBox const *box );

vec2u16 widget_border_get_size( struct WidgetBox const *box );
vec2u16 widget_content_get_size( struct WidgetBox const *box );
