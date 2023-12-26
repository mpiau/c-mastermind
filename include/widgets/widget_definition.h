#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "widgets/widget.h"
#include "console.h"
#include "mouse.h"

enum WidgetTruncatedStatus
{
    WidgetTruncatedStatus_NONE   		= 0x000,
    WidgetTruncatedStatus_X_AXIS 		= 0x001,
    WidgetTruncatedStatus_Y_AXIS 		= 0x010,
    WidgetTruncatedStatus_OUT_OF_BOUNDS = 0x100,
};

enum WidgetVisibilityStatus
{
    WidgetVisibilityStatus_VISIBLE,
    WidgetVisibilityStatus_HIDDEN
};

enum WidgetBorderOption
{
    WidgetBorderOption_INVISIBLE,
    WidgetBorderOption_VISIBLE_ON_TRUNCATE,
    WidgetBorderOption_ALWAYS_VISIBLE,
};


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

struct WidgetCallbacks
{
    WidgetFrameCallback		frameCb;
    WidgetClearCallback		clearCb;
    WidgetRedrawCallback	redrawCb;
    WidgetMouseMoveCallback mouseMoveCb;
    // [...] 
};

struct Widget
{
    enum WidgetId	 			id;
    struct WidgetBox 			box;
    struct WidgetCallbacks	    callbacks;
    enum WidgetVisibilityStatus	visibilityStatus;
    bool                        redrawNeeded;
};
