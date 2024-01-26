#include "mouse.h"

#include "gameloop.h"
#include "events.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

enum // Constants
{
    CALLBACKS_MAX_COUNT = 4
};

struct MouseInfo
{
    screenpos position;

    OnMouseMoveCallback moveCallbacks[CALLBACKS_MAX_COUNT];
    u32 moveCallbackCount;
};

static struct MouseInfo s_mouseInfo = {};


screenpos mouse_pos()
{
    return s_mouseInfo.position;
}


static void mouse_moved( vec2u16 const mousePos )
{
    screenpos const oldPos = mouse_pos();
	screenpos const newPos = (screenpos) { .x = mousePos.x + 1, .y = mousePos.y + 1 };

	if ( oldPos.x == newPos.x && oldPos.y == newPos.y )
        return;

    s_mouseInfo.position = newPos;

    struct Event mouseMoved = (struct Event) {
        .type = EventType_MOUSE_MOVED,
        .mouseMoved = (struct EventMouseMoved) {
            .pos = s_mouseInfo.position
        }
    };
    event_trigger( &mouseMoved );

/*    for ( u32 idx = 0; idx < s_mouseInfo.moveCallbackCount; ++idx )
    {
        s_mouseInfo.moveCallbacks[idx]( s_mouseInfo.position );
    }*/
}


void mouse_consume_event( struct _MOUSE_EVENT_RECORD const *mouseEvent )
{
	// If the mouse moved but didn't move enough to change its coordinates on the screen,
	// the event won't be sent. However, the MOUSE_MOVED won't necessarily be sent with it, so do not encapsulate
	// the move condition in it.
	mouse_moved( *(vec2u16 *)&mouseEvent->dwMousePosition );

    if ( mouseEvent->dwEventFlags == MOUSE_WHEELED )
    {
        // Note: 
        // Just for testing. We should need to emit a Scroll keyinput, and the board would need to check that the mouse is hovering the board.
        // But it allow me to test the scroll with the main board for the moment so it's nice for testing

        // Comparaison based on the Windows documentation :
        // If the high word of the dwButtonState member contains a positive value, the wheel was rotated forward, away from the user.
        // Otherwise, the wheel was rotated backward, toward the user.
        if ( (short)HIWORD( mouseEvent->dwButtonState ) > 0 )
            gameloop_emit_key( KeyInput_ARROW_UP );
        else
            gameloop_emit_key( KeyInput_ARROW_DOWN );
        return;
    }

	if ( mouseEvent->dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED )
	{
        gameloop_emit_key( KeyInput_MOUSE_BTN_LEFT );
	}
	else if ( mouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED )
	{
        gameloop_emit_key( KeyInput_MOUSE_BTN_RIGHT );
	}
}


bool mouse_register_on_mouse_move_callback( OnMouseMoveCallback const callback )
{
    if ( callback == NULL )                                     return false;
    if ( s_mouseInfo.moveCallbackCount == CALLBACKS_MAX_COUNT ) return false;

    s_mouseInfo.moveCallbacks[s_mouseInfo.moveCallbackCount] = callback;
    s_mouseInfo.moveCallbackCount++;
    return true;
}


bool mouse_init( void )
{
	s_mouseInfo = (struct MouseInfo) {};
    return true;
}
