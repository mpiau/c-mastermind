#include "mouse.h"

#include <windows.h>

#include "console.h"


enum // Constants
{
    CALLBACKS_MAX_COUNT = 4
};

struct MouseInfo
{
    screenpos position;

    OnMouseMoveCallback moveCallbacks[CALLBACKS_MAX_COUNT];
    u32 moveCallbackCount;

    OnMouseClickCallback clickCallbacks[CALLBACKS_MAX_COUNT];
    u32 clickCallbackCount;
};

static struct MouseInfo s_mouseInfo = {};


screenpos mouse_get_position()
{
    return s_mouseInfo.position;
}


static void mouse_moved( vec2u16 const mousePos )
{
    screenpos const oldPos = mouse_get_position();
	screenpos const newPos = SCREENPOS( mousePos.x + 1, mousePos.y + 1 );

	if ( oldPos.x == newPos.x && oldPos.y == newPos.y ) return;

    s_mouseInfo.position = newPos;

    for ( u32 idx = 0; idx < s_mouseInfo.moveCallbackCount; ++idx )
    {
        s_mouseInfo.moveCallbacks[idx]( oldPos, s_mouseInfo.position );
    }
}


static void call_click_callback( enum MouseButton const button )
{
	screenpos const mousePos = mouse_get_position();

	for ( u32 idx = 0; idx < s_mouseInfo.clickCallbackCount; ++idx )
    {
        s_mouseInfo.clickCallbacks[idx]( mousePos, button );
    }
}


void mouse_consume_event( struct _MOUSE_EVENT_RECORD const *mouseEvent )
{
	// If the mouse moved but didn't move enough to change its coordinates on the screen,
	// the event won't be sent. However, the MOUSE_MOVED won't necessarily be sent with it, so do not encapsulate
	// the move condition in it.
	mouse_moved( *(vec2u16 *)&mouseEvent->dwMousePosition );

	/*switch( mouseEvent->dwEventFlags )
	{
		case MOUSE_MOVED:
			return;

		case MOUSE_HWHEELED:
		case MOUSE_WHEELED:
		case DOUBLE_CLICK:
			return;
	}*/

	if ( mouseEvent->dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED )
	{
		call_click_callback( MouseButton_LeftClick );
	}
	else if ( mouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED )
	{
		call_click_callback( MouseButton_RightClick );
	}
}


bool mouse_register_on_mouse_mouvement_callback( OnMouseMoveCallback const callback )
{
    if ( callback == NULL )                                     return false;
    if ( s_mouseInfo.moveCallbackCount == CALLBACKS_MAX_COUNT ) return false;

    s_mouseInfo.moveCallbacks[s_mouseInfo.moveCallbackCount] = callback;
    s_mouseInfo.moveCallbackCount++;
    return true;
}


bool mouse_register_on_mouse_click_callback( OnMouseClickCallback const callback )
{
    if ( callback == NULL )                                      return false;
    if ( s_mouseInfo.clickCallbackCount == CALLBACKS_MAX_COUNT ) return false;

    s_mouseInfo.clickCallbacks[s_mouseInfo.clickCallbackCount] = callback;
    s_mouseInfo.clickCallbackCount++;
    return true;	
}


void mouse_init( void )
{
	s_mouseInfo = (struct MouseInfo) {};
}