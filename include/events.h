#pragma once

#include "core/core.h"

enum EventType
{
    EventType_STOP_EXECUTION,

    EventType_NEW_GAME,
    EventType_ABANDON_GAME,
    EventType_GAME_LOST,
    EventType_GAME_WON,

    EventType_OPEN_SETTINGS_MENU,
    EventType_CLOSE_SETTINGS_MENU,

    EventType_PEG_SELECTED,
    EventType_PEG_UNSELECTED,
    EventType_PEG_ADDED,
    EventType_PEG_REMOVED,

    EventType_NEXT_TURN,
};


struct EventPegAdded
{
    usize index;
    // peg
};

struct EventData
{
    union
    {
        struct EventPegAdded peg_added_event;
    };
};

