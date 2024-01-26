#include "events.h"

enum // Constants
{
    MAX_SUBSCRIBERS_COUNT = 16
};

struct Subscription
{
    void *subscriber;
    EventTriggeredCb callback;
    enum EventType subscribedEvents;
};

static struct Subscription s_subscriptions[MAX_SUBSCRIBERS_COUNT];


static struct Subscription *tryget_existing( void const *subscriber )
{
    assert( subscriber );

    for ( usize idx = 0; idx < MAX_SUBSCRIBERS_COUNT; ++idx )
    {
        struct Subscription *sub = &s_subscriptions[idx];
        if ( sub->subscriber == subscriber )
        {
            return sub;
        }
    }

    return NULL;
}

static struct Subscription *tryget_next_available( void )
{
    for ( usize idx = 0; idx < MAX_SUBSCRIBERS_COUNT; ++idx )
    {
        struct Subscription *sub = &s_subscriptions[idx];
        if ( sub->subscriber == NULL )
        {
            return sub;
        }
    }

    return NULL;
}

static struct Subscription *tryget( void const *subscriber )
{
    struct Subscription *sub = tryget_existing( subscriber );
    return sub ? sub : tryget_next_available();
}


bool event_register( void *subscriber, EventTriggeredCb const callback )
{
    struct Subscription *sub = tryget_existing( subscriber );
    if ( sub )
    {
        sub->callback = callback;
        return true;
    }

    sub = tryget_next_available();
    if ( !sub ) return false;

    *sub = (struct Subscription ) {
        .subscriber = subscriber,
        .callback = callback,
        .subscribedEvents = EventType_MaskNone
    };

    return true;
}


void event_unregister( void *subscriber )
{
    struct Subscription *sub = tryget_existing( subscriber );
    if ( sub )
    {
        sub->subscriber = NULL;
        sub->callback = NULL;
        sub->subscribedEvents = EventType_MaskNone;
    }
}


bool event_subscribe( void *subscriber, enum EventType events )
{
    struct Subscription *sub = tryget( subscriber );
    if ( sub )
    {
        sub->subscribedEvents |= events;
        return true;
    }
    return false;
}


bool event_subscribe_all( void *subscriber )
{
    struct Subscription *sub = tryget( subscriber );
    if ( sub )
    {
        sub->subscribedEvents |= EventType_MaskAll;
        return true;
    }
    return false;
}


void event_unsubscribe( void *subscriber, enum EventType const events )
{
    struct Subscription *sub = tryget( subscriber );
    if ( sub )
    {
        sub->subscribedEvents &= ~events;
    }
}


void event_unsubscribe_all( void *subscriber )
{
    struct Subscription *sub = tryget( subscriber );
    if ( sub )
    {
        sub->subscribedEvents = EventType_MaskNone;
    }
}


void event_trigger( struct Event const *event )
{
    assert( event );

    for ( int idx = MAX_SUBSCRIBERS_COUNT - 1; idx >= 0; --idx )
    {
        struct Subscription *sub = &s_subscriptions[idx];
        if ( !sub->subscriber || ( sub->subscribedEvents & event->type ) == 0 ) continue;

        assert( sub->callback );
        assert( sub->subscriber );

        enum EventPropagation const propagate = sub->callback( sub->subscriber, event );
        if ( propagate == EventPropagation_STOP ) break;
    }    
}
