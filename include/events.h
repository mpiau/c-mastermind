#pragma once

#include "core/core.h"
#include "keyboard_inputs.h"
#include "game/piece.h"

enum EventType
{
    EventType_STOP_EXECUTION = 0b0000000000000001,
    EventType_SCREEN_RESIZED = 0b0000000000000010,
    EventType_GAME_NEW       = 0b0000000000000100,
    EventType_GAME_LOST      = 0b0000000000001000,
    EventType_GAME_WON       = 0b0000000000010000,
    EventType_PEG_SELECTED   = 0b0000000000100000,
    EventType_PEG_UNSELECTED = 0b0000000001000000,
    EventType_PEG_ADDED      = 0b0000000010000000,
    EventType_PEG_REMOVED    = 0b0000000100000000,
    EventType_USER_INPUT     = 0b0000001000000000,
    EventType_MOUSE_MOVED    = 0b0000010000000000,
    EventType_NEXT_TURN      = 0b0000100000000000,

    EventType_MaskNone       = 0b0000000000000000,
    EventType_MaskAll        = 0b1111111111111111
};

enum EventPropagation 
{
    EventPropagation_STOP     = 0,
    EventPropagation_CONTINUE = 1
};


struct EventPegAdded
{
    usize index;
    usize turn;
    gamepiece piece;
};

struct EventPegRemoved
{
    usize index;
    usize turn;
    gamepiece piece;
};

struct EventUserInput
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

struct EventGameNew
{
    usize nbTurns;
    usize nbPegsPerTurn;
};

struct Event
{
    enum EventType type;
    union
    {
        struct EventPegAdded pegAdded;
        struct EventPegRemoved pegRemoved;
        struct EventUserInput userInput;
        struct EventMouseMoved mouseMoved;
        struct EventScreenResized screenResized;
        struct EventGameNew newGame;
    };
};


struct EventData {};

#define EVENT_INPUT( _input )                   \
    ( (struct Event) {                          \
        .type = EventType_USER_INPUT,           \
        .userInput = (struct EventUserInput) {  \
            .input = _input                     \
        }                                       \
    } )

typedef enum EventPropagation ( *EventTriggeredCb )( void *subscriber, struct Event const *event );

bool event_register( void *subscriber, EventTriggeredCb const callback );
void event_unregister( void *subscriber );

bool event_subscribe( void *subscriber, enum EventType events );
bool event_subscribe_all( void *subscriber );
void event_unsubscribe( void *subscriber, enum EventType events );
void event_unsubscribe_all( void *subscriber );

void event_trigger( struct Event const *event );
