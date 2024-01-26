#include "requests.h"
#include "mastermind.h"
#include "gameloop.h"

void request_send( struct Request const *request )
{
    // KISS for the moment.
    if ( mastermind_on_request( request ) == RequestStatus_TREATED ) return;
    if ( gameloop_on_request( request ) == RequestStatus_TREATED ) return;
}
