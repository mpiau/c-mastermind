#pragma once

#include "core_types.h"
#include "core_unions.h"

enum ComponentId
{
    ComponentId_FRAMERATE,
	ComponentId_SCREEN_SIZE,
    ComponentId_MOUSE_POSITION,
	ComponentId_GAME_BUTTONS,
    ComponentId_BOARD,
    ComponentId_COUNTDOWN,
    ComponentId_TIMER,
    ComponentId_SUMMARY,
    ComponentId_PEG_SELECTION,

    ComponentId_Count,
};

// Forward declaration
enum KeyInput;
struct ComponentHeader;


bool components_init( void );
void components_uninit( void );

// Functions that have an impact on all components

void components_on_screen_resize( screensize size );
void components_frame( void );
bool components_try_consume_input( enum KeyInput input );

// Functions specific to a component

bool component_exists( enum ComponentId id );
struct ComponentHeader *component_try_get( enum ComponentId id ); // return NULL if doesn't exist

void component_enable( enum ComponentId id );
void component_disable( enum ComponentId id );

// This should be internal to all components, but not shared in this header file.
void component_make_header( struct ComponentHeader *widget, enum ComponentId id, bool enabled );
