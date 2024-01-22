#pragma once

#include "components/components.h"

// Forward declaration
enum MouseButton;
enum GameUpdateType;
enum KeyInput;

// Callbacks definition
typedef void ( *ComponentCbEnable )( struct ComponentHeader *header );
typedef void ( *ComponentCbDisable )( struct ComponentHeader *header );

typedef void ( *ComponentCbFrame )( struct ComponentHeader *header );
typedef void ( *ComponentCbRefresh )( struct ComponentHeader const *header );
typedef void ( *ComponentCbMouseMove )( struct ComponentHeader *header, screenpos pos );
typedef void ( *WidgetMouseClickCallback )( struct ComponentHeader *header, screenpos pos, enum MouseButton button );
typedef void ( *ComponentCbResize )( struct ComponentHeader *header, screensize newSize );
typedef void ( *ComponentCbEventReceived )( struct ComponentHeader *header, enum EventType event, struct EventData const *data );
// Must return true if the input has been consumed by the component. False otherwise.
typedef bool ( *ComponentCbInputReceived )( struct ComponentHeader *header, enum KeyInput input );


struct ComponentCallbacks
{
    ComponentCbEnable enableCb;
    ComponentCbDisable disableCb;

    ComponentCbFrame frameCb;
    ComponentCbRefresh refreshCb;
    ComponentCbMouseMove mouseMoveCb;
	WidgetMouseClickCallback mouseClickCb;
	ComponentCbResize resizeCb;
    ComponentCbEventReceived eventReceivedCb;
    ComponentCbInputReceived inputReceivedCb;
};


struct ComponentHeader
{
    enum ComponentId id;

    bool enabled;
    bool refreshNeeded;
    struct ComponentCallbacks callbacks;
};
#define CAST_TO_COMP_HEADER( _comp ) ( ( struct ComponentHeader * )( _comp ) )
