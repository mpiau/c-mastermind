#pragma once 

#include "game/piece.h"


enum RequestType
{
    RequestType_START_NEW_GAME,
    RequestType_ABANDON_GAME,

    RequestType_REVEAL_SOLUTION,
    RequestType_HIDE_SOLUTION,

    RequestType_PEG_SELECT,
    RequestType_PEG_UNSELECT,

    RequestType_PEG_ADD,
    RequestType_PEG_REMOVE,

    RequestType_CONFIRM_TURN,
    RequestType_RESET_TURN,

    RequestType_NEXT,
    RequestType_PREVIOUS,

    RequestType_EXIT_APP,
};

enum RequestStatus
{
    RequestStatus_SKIPPED,
    RequestStatus_TREATED
};

struct RequestPeg
{
    enum PegId id;
};

struct Request
{
    enum RequestType type;
    union
    {
        struct RequestPeg peg;
    };
};

void request_send( struct Request const *request );
