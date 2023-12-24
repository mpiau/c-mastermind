#pragma once

#include "core_types.h"

// Widgets should be graphical only !!
// No widget -> the game can still run the same
// Ex: removing the fps bar widgte doesn't stop the game from calculating the FPS.

enum WidgetId
{
    WidgetId_GAME,
    WidgetId_FPS_BAR,
    WidgetId_BOARD,
    WidgetId_BOARD_SUMMARY,
    WidgetId_TIMER,
    WidgetId_COUNTDOWN,
    WidgetId_RULES,
    WidgetId_OPTIONS_MENU,
    WidgetId_PAUSE_MENU,
    // WidgetId_SCORE ?

    WidgetId_Count,
};


bool widget_global_init( void );
void widget_global_uninit( void );

bool widget_exists( enum WidgetId id );
struct Widget *widget_optget( enum WidgetId id ); // return NULL if doesn't exist

void widget_frame( void );
