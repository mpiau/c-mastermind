#pragma once 


enum RequestType
{
    RequestType_START_NEW_GAME,
    RequestType_ABANDON_GAME,

    RequestType_REVEAL_SOLUTION,
    RequestType_HIDE_SOLUTION,

    RequestType_EXIT_APP,
};

enum RequestStatus
{
    RequestStatus_SKIPPED,
    RequestStatus_TREATED
};


struct Request
{
    enum RequestType type;
};

void request_send( struct Request const *request );
