#pragma once

#include "core_types.h"

struct FPSCounter;

bool widget_fpsbar_init( struct FPSCounter *fpsCounter );
void widget_fpsbar_uninit( void );

void widget_fpsbar_show( void );
void widget_fpsbar_show_fps( void );
void widget_fpsbar_show_ms( void );

void widget_fpsbar_hide( void );
void widget_fpsbar_hide_fps( void );
void widget_fpsbar_hide_ms( void );

// Hide only stop drawing the widget, remove free up the space in the screen (thus can be used by other widgets)
void widget_fpsbar_remove( void );


// void widget_fpsbar_on_screen_resized(), to hook on a future screen size handler ?

// struct WidgetFPSBar *widget_fpsbar_get_instance( void );
