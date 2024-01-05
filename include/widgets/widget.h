#pragma once

#include "core_types.h"

// Widgets should be graphical only !!
// No widget -> the game can still run the same
// Ex: removing the fps bar widgte doesn't stop the game from calculating the FPS.

enum ComponentId
{
    ComponentId_FRAMERATE,
	ComponentId_SCREEN_SIZE,
	ComponentId_GAME_BUTTONS,
    ComponentId_BOARD,
    ComponentId_COUNTDOWN,
    ComponentId_TIMER,
    ComponentId_SUMMARY,
    ComponentId_PEG_SELECTION,

    ComponentId_Count,
};

// Forward declaration
struct Widget;
enum KeyInput;

bool widget_global_init( void );
void widget_global_uninit( void );

bool widget_exists( enum ComponentId id );

struct Widget *widget_optget( enum ComponentId id ); // return NULL if doesn't exist

void widget_frame( void );
bool widget_try_consume_input( enum KeyInput input );

void widget_set_header( struct Widget *widget, enum ComponentId id, bool enabled );

