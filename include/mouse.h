#pragma once

#include "core_types.h"
#include "core_unions.h"

enum MouseButton
{
	MouseButton_LEFT_CLICK,
	MouseButton_RIGHT_CLICK
};

typedef void ( *OnMouseClickCallback )( screenpos_deprecated pos, enum MouseButton button );
typedef void ( *OnMouseMoveCallback ) ( screenpos_deprecated oldPos, screenpos_deprecated newPos );

// The mouse position is different from the screen coordinate
// While it would be x=1 and y=1 for the screen buffer
// It would be x=0 and y=0 for the mouse position.
// This function transform the mouse coordinates into screenpos_deprecated to simplify its usage
screenpos_deprecated mouse_get_position();

struct _MOUSE_EVENT_RECORD;
void mouse_consume_event( struct _MOUSE_EVENT_RECORD const *mouseEvent );
void mouse_init( void );

bool mouse_register_on_mouse_mouvement_callback( OnMouseMoveCallback const callback );
bool mouse_register_on_mouse_click_callback( OnMouseClickCallback const callback );
