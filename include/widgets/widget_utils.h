#pragma once

#include "widget_definition.h"
#include "console/console.h"

void widget_utils_draw_borders( struct WidgetBox const *box );
void widget_utils_clear_content( struct WidgetBox *box );

void widget_utils_set_title( struct WidgetBox *box, utf16 const *title, enum ConsoleColorFG color );

void widget_utils_calculate_truncation( struct WidgetBox *box, screenpos screenSize );
void widget_utils_set_position( struct WidgetBox *box, screenpos borderUpLeft, vec2u16 contentSize );
