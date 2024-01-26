#pragma once

#include "core/core.h"

typedef void ( *OnMouseMoveCallback ) ( screenpos pos );


struct _MOUSE_EVENT_RECORD;


bool mouse_init( void );

// The mouse position is different from the screen coordinate
// While it would be x=1 and y=1 for the screen buffer
// It would be x=0 and y=0 for the mouse position.
// This function transform the mouse coordinates into screenpos to simplify its usage
screenpos mouse_pos();
void mouse_consume_event( struct _MOUSE_EVENT_RECORD const *mouseEvent );
