#pragma once

#include "core_types.h"

struct Widget *widget_fpsbar_create( void );

// You MUST give the widget given by the create function.
// If you give another type of widget, the code will assert.
void widget_fpsbar_show( struct Widget *widget );
void widget_fpsbar_show_fps( struct Widget *widget );
void widget_fpsbar_show_ms( struct Widget *widget );

void widget_fpsbar_hide( struct Widget *widget ); // hide content maybe ?
void widget_fpsbar_hide_fps( struct Widget *widget );
void widget_fpsbar_hide_ms( struct Widget *widget );

bool widget_is_shown( struct Widget *widget );
bool widget_is_fps_shown( struct Widget *widget );
bool widget_is_ms_shown( struct Widget *widget );

// Note: One Toggle function instead of show + hide ?
