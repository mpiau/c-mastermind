#pragma once

#include "core/core.h"
#include "keyboard_inputs.h"

enum EventType
{
    EventType_STOP_EXECUTION = 0b0000000000000001,
    EventType_SCREEN_RESIZED = 0b0000000000000010,
    EventType_NEW_GAME       = 0b0000000000000100,
    EventType_ABANDON_GAME   = 0b0000000000001000,
    EventType_GAME_LOST      = 0b0000000000010000,
    EventType_GAME_WON       = 0b0000000000100000,
    EventType_PEG_SELECTED   = 0b0000000001000000,
    EventType_PEG_UNSELECTED = 0b0000000010000000,
    EventType_PEG_ADDED      = 0b0000000100000000,
    EventType_PEG_REMOVED    = 0b0000001000000000,
    EventType_KEY_PRESSED    = 0b0000010000000000,
    EventType_MOUSE_MOVED    = 0b0000100000000000,
    EventType_NEXT_TURN      = 0b0001000000000000,

    EventType_MaskNone       = 0b0000000000000000,
    EventType_MaskAll        = 0b0001111111111111
};

enum EventPropagation 
{
    EventPropagation_STOP     = 0,
    EventPropagation_CONTINUE = 1
};


struct EventPegAdded
{
    usize index;
    // peg
};


struct EventKeyPressed
{
    enum KeyInput input;
};


struct EventMouseMoved
{
    screenpos pos;
};


struct EventScreenResized
{
    screensize size;
};


struct Event
{
    enum EventType type;
    union
    {
        struct EventPegAdded pegAddedEvent;
        struct EventKeyPressed keyPressedEvent;
        struct EventMouseMoved mouseMoved;
        struct EventScreenResized screenResized;
    };
};


struct EventData {};

typedef enum EventPropagation ( *EventTriggeredCb )( void *subscriber, struct Event const *event );

bool event_register( void *subscriber, EventTriggeredCb const callback );
void event_unregister( void *subscriber );

bool event_subscribe( void *subscriber, enum EventType events );
bool event_subscribe_all( void *subscriber );
void event_unsubscribe( void *subscriber, enum EventType events );
void event_unsubscribe_all( void *subscriber );

void event_trigger( struct Event const *event );




enum RequestType
{
    RequestType_START_NEW_GAME,
    RequestType_ABANDON_GAME,

    RequestType_REVEAL_SOLUTION,
    RequestType_HIDE_SOLUTION,

    RequestType_EXIT_APP,
};

void request_send( enum RequestType type );


//void event_trigger( enum EventType type, struct EventData *data );
