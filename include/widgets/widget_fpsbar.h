#pragma once

#include "core_types.h"

struct WidgetFPSBar;
struct FPSCounter;

bool widget_fpsbar_init( struct FPSCounter *fpsCounter );
void widget_fpsbar_uninit( void );

void widget_fpsbar_frame( void );
// void widget_fpsbar_on_screen_resized(), to hook on a future screen size handler ?

// struct WidgetFPSBar *widget_fpsbar_get_instance( void );
