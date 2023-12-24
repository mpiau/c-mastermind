#pragma once

#include "core_types.h"
#include "core_unions.h"

typedef void ( *OnMouseMovementCallback )( screenpos oldPos, screenpos newPos );

// The mouse position is different from the screen coordinate
// While it would be x=1 and y=1 for the screen buffer
// It would be x=0 and y=0 for the mouse position.
// This function transform the mouse coordinates into screenpos to simplify its usage
screenpos mouse_get_position();

void mouse_update_position( vec2u16 newPos );
bool mouse_register_on_mouse_mouvement_callback( OnMouseMovementCallback const callback );
