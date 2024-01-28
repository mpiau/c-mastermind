#pragma once

#include "core/core.h"
#include "keyboard_inputs.h"
#include "game/piece.h"

enum EventType
{
    EventType_STOP_EXECUTION    = 0b0000000000000001,
    EventType_SCREEN_RESIZED    = 0b0000000000000010,
    EventType_GAME_NEW          = 0b0000000000000100,
    EventType_GAME_LOST         = 0b0000000000001000,
    EventType_GAME_WON          = 0b0000000000010000,
    EventType_PEG_SELECTED      = 0b0000000000100000,
    EventType_PEG_UNSELECTED    = 0b0000000001000000,
    EventType_PEG_ADDED         = 0b0000000010000000,
    EventType_PEG_REMOVED       = 0b0000000100000000,
    EventType_USER_INPUT        = 0b0000001000000000,
    EventType_MOUSE_MOVED       = 0b0000010000000000,
    EventType_NEW_TURN          = 0b0000100000000000,
    EventType_PIN_ADDED         = 0b0001000000000000,
    EventType_PIN_REMOVED       = 0b0010000000000000,
    EventType_PEG_REVEALED      = 0b0100000000000000,
    EventType_PEG_HIDDEN        = 0b1000000000000000,

    EventType_MaskNone          = 0b0000000000000000,
    EventType_MaskAll           = 0b1111111111111111
};

enum EventPropagation 
{
    EventPropagation_STOP     = 0,
    EventPropagation_CONTINUE = 1
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
    gamepiece defaultPeg;
    gamepiece defaultPin;
};

struct EventSolution
{
    gamepiece *pieces;
    usize nbPieces;
};

struct EventPeg
{
    usize turn;
    usize index;
    gamepiece piece;
};

struct EventPin
{
    usize turn;
    usize index;
    gamepiece piece;
};

struct EventNewTurn
{
    usize turn;
};

struct Event
{
    enum EventType type;
    union
    {
        struct EventPeg peg;
        struct EventPin pin;
        struct EventNewTurn newTurn;
        struct EventUserInput userInput;
        struct EventMouseMoved mouseMoved;
        struct EventScreenResized screenResized;
        struct EventGameNew newGame;
        struct EventSolution solution;
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

#define EVENT_GAME_NEW( _nbTurns, _nbPegsPerTurn ) \
    ( (struct Event) {                             \
        .type = EventType_GAME_NEW,                \
        .newGame = (struct EventGameNew) {         \
            .nbTurns = _nbTurns,                   \
            .nbPegsPerTurn = _nbPegsPerTurn,       \
        }                                          \
    } )

#define EVENT_PEG( _event, _turn, _index, _piece ) \
    ( (struct Event) {                             \
        .type = _event,                            \
        .peg = (struct EventPeg) {                 \
            .turn = _turn,                         \
            .index = _index,                       \
            .piece = _piece                        \
        }                                          \
    } )

#define EVENT_PIN( _event, _turn, _index, _piece ) \
    ( (struct Event) {                             \
        .type = _event,                            \
        .pin = (struct EventPin) {                 \
            .turn = _turn,                         \
            .index = _index,                       \
            .piece = _piece                        \
        }                                          \
    } )

typedef enum EventPropagation ( *EventTriggeredCb )( void *subscriber, struct Event const *event );

bool event_register( void *subscriber, EventTriggeredCb const callback );
void event_unregister( void *subscriber );

bool event_subscribe( void *subscriber, enum EventType events );
bool event_subscribe_all( void *subscriber );
void event_unsubscribe( void *subscriber, enum EventType events );
void event_unsubscribe_all( void *subscriber );

void event_trigger( struct Event const *event );
