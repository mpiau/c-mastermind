#include "mouse.h"

enum // Constants
{
    CALLBACKS_MAX_COUNT = 10
};


struct MouseInfo
{
    screenpos position;
    OnMouseMovementCallback callbacks[CALLBACKS_MAX_COUNT];
    u32 callbackCount;
};

static struct MouseInfo s_mouseInfo = {};


screenpos mouse_get_position()
{
    return s_mouseInfo.position;
}


void mouse_update_position( vec2u16 const newPos )
{
    screenpos const oldPos = mouse_get_position();

    s_mouseInfo.position = (screenpos) {
        .x = newPos.x + 1,
        .y = newPos.y + 1
    };

    for ( u32 idx = 0; idx < s_mouseInfo.callbackCount; ++idx )
    {
        s_mouseInfo.callbacks[idx]( oldPos, s_mouseInfo.position );
    }
}


bool mouse_register_on_mouse_mouvement_callback( OnMouseMovementCallback const callback )
{
    if ( callback == NULL )                                 return false;
    if ( s_mouseInfo.callbackCount == CALLBACKS_MAX_COUNT ) return false;

    s_mouseInfo.callbacks[s_mouseInfo.callbackCount] = callback;
    s_mouseInfo.callbackCount++;
    return true;
}
