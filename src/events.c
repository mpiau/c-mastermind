#include "events.h"

enum // Constants
{
    MAX_WIDGETS_COUNT = 16
};

struct WidgetSubscription
{
    struct Widget *widget;
    EventTriggeredCb callback;
    enum EventType subscribedEvents;
};

static struct WidgetSubscription s_subscriptions[MAX_WIDGETS_COUNT];


static struct WidgetSubscription *tryget_existing( struct Widget const *widget )
{
    assert( widget );

    for ( usize idx = 0; idx < MAX_WIDGETS_COUNT; ++idx )
    {
        struct WidgetSubscription *sub = &s_subscriptions[idx];
        if ( sub->widget == widget )
        {
            return sub;
        }
    }

    return NULL;
}

static struct WidgetSubscription *tryget_next_available( void )
{
    for ( usize idx = 0; idx < MAX_WIDGETS_COUNT; ++idx )
    {
        struct WidgetSubscription *sub = &s_subscriptions[idx];
        if ( sub->widget == NULL )
        {
            return sub;
        }
    }

    return NULL;
}

static struct WidgetSubscription *tryget( struct Widget const *widget )
{
    struct WidgetSubscription *sub = tryget_existing( widget );
    return sub ? sub : tryget_next_available();
}


bool event_register( struct Widget *widget, EventTriggeredCb const callback )
{
    struct WidgetSubscription *sub = tryget_existing( widget );
    if ( sub )
    {
        sub->callback = callback;
        return true;
    }

    sub = tryget_next_available();
    if ( !sub ) return false;

    *sub = (struct WidgetSubscription ) {
        .widget = widget,
        .callback = callback,
        .subscribedEvents = EventType_MaskNone
    };

    return true;
}


void event_unregister( struct Widget *widget )
{
    struct WidgetSubscription *sub = tryget_existing( widget );
    if ( sub )
    {
        sub->widget = NULL;
        sub->callback = NULL;
        sub->subscribedEvents = EventType_MaskNone;
    }
}


bool event_subscribe( struct Widget *widget, enum EventType events )
{
    struct WidgetSubscription *sub = tryget( widget );
    if ( sub )
    {
        sub->subscribedEvents |= events;
        return true;
    }
    return false;
}


bool event_subscribe_all( struct Widget *widget )
{
    struct WidgetSubscription *sub = tryget( widget );
    if ( sub )
    {
        sub->subscribedEvents |= EventType_MaskAll;
        return true;
    }
    return false;
}


void event_unsubscribe( struct Widget *widget, enum EventType events )
{
    struct WidgetSubscription *sub = tryget( widget );
    if ( sub )
    {
        sub->subscribedEvents &= ~events;
    }
}


void event_unsubscribe_all( struct Widget *widget )
{
    struct WidgetSubscription *sub = tryget( widget );
    if ( sub )
    {
        sub->subscribedEvents = EventType_MaskNone;
    }
}


void event_trigger( struct Event const *event )
{
    assert( event );

    for ( usize idx = 0; idx < MAX_WIDGETS_COUNT; ++idx )
    {
        struct WidgetSubscription *sub = &s_subscriptions[idx];
        if ( sub->subscribedEvents & event->type )
        {
            sub->callback( sub->widget, event );
        }
    }    
}
