#pragma once

#include "core_types.h"

// Widgets should be graphical only !!
// No widget -> the game can still run the same
// Ex: removing the fps bar widgte doesn't stop the game from calculating the FPS.

enum WidgetId
{
    WidgetId_FRAMERATE,
	WidgetId_SCREEN_SIZE,
	WidgetId_BOARD_BUTTONS,
    WidgetId_BOARD,
    WidgetId_COUNTDOWN,
    WidgetId_TIMER,
    WidgetId_BOARD_SUMMARY,
    WidgetId_PEG_SELECTION,

    WidgetId_Count,
};

// Forward declaration
struct Widget;
enum KeyInput;

bool widget_global_init( void );
void widget_global_uninit( void );

bool widget_exists( enum WidgetId id );

struct Widget *widget_optget( enum WidgetId id ); // return NULL if doesn't exist

void widget_frame( void );
bool widget_try_consume_input( enum KeyInput input );


