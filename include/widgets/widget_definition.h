#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "widgets/widget.h"
#include "mastermind.h"
#include "console.h"
#include "mouse.h"
#include "rect.h"

enum WidgetTruncatedStatus
{
    WidgetTruncatedStatus_NONE   		= 0x000,
    WidgetTruncatedStatus_X_AXIS 		= 0x001,
    WidgetTruncatedStatus_Y_AXIS 		= 0x010,
    WidgetTruncatedStatus_OUT_OF_BOUNDS = 0x100,
};


enum WidgetBorderOption
{
    WidgetBorderOption_INVISIBLE,
    WidgetBorderOption_VISIBLE_ON_TRUNCATE,
    WidgetBorderOption_ALWAYS_VISIBLE,
};

// Transition on the widgetBox, I don't think this is a good idea for a terminal base game, it takes too much space.
struct WidgetBox
{
    // Screen positions
    screenpos borderUpLeft;
    screenpos borderBottomRight;
    screenpos contentUpLeft;
    screenpos contentBottomRight;

    // Truncation related
    screenpos truncatedBorderBottomRight;
    screenpos truncatedContentBottomRight;
    enum WidgetTruncatedStatus truncatedStatus;

    // Title
    utf16 const *borderOptionalTitle;
    enum ConsoleColorFG borderTitleColor;

    // Border customization
    enum ConsoleColorFG borderColor;
    enum WidgetBorderOption borderOption;
};


typedef void ( *WidgetFrameCallback )( struct Widget *widget );
typedef void ( *WidgetRedrawCallback )( struct Widget *widget );
typedef void ( *WidgetClearCallback )( struct Widget *widget );
typedef void ( *WidgetMouseMoveCallback )( struct Widget *widget, screenpos oldPos, screenpos newPos );
typedef void ( *WidgetMouseClickCallback )( struct Widget *widget, screenpos pos, enum MouseButton button );
typedef void ( *WidgetResizeCallback )( struct Widget *widget, vec2u16 oldSize, vec2u16 newSize );
typedef void ( *WidgetGameUpdateCallback )( struct Widget *widget, struct Mastermind const *mastermind, enum GameUpdateType type );

// Must return true if the input has been consumed by the widget. False otherwise.
typedef bool ( *WidgetOnInputReceivedCallback )( struct Widget *widget, enum KeyInput input );

struct WidgetCallbacks
{
    WidgetFrameCallback		 frameCb;
    WidgetClearCallback		 clearCb;
    WidgetRedrawCallback	 redrawCb;
    WidgetMouseMoveCallback  mouseMoveCb;
	WidgetMouseClickCallback mouseClickCb;
	WidgetResizeCallback	 resizeCb;
    WidgetGameUpdateCallback gameUpdateCb;
    WidgetOnInputReceivedCallback inputReceivedCb;
};

struct Widget
{
    enum WidgetId	 	   id;
	struct Rect            rectBox;
    struct WidgetBox 	   box;
    struct WidgetCallbacks callbacks;
    bool                   enabled;
    bool                   redrawNeeded;
};
