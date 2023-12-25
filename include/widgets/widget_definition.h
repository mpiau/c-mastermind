#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "widgets/widget.h"
#include "console.h"
#include "mouse.h"

enum WidgetTruncate
{
    WidgetTruncate_NONE   = 0x00,
    WidgetTruncate_X_AXIS = 0x01,
    WidgetTruncate_Y_AXIS = 0x10
};

enum WidgetBorderOption
{
    WidgetBorderOption_INVISIBLE,
    WidgetBorderOption_VISIBLE_ON_TRUNCATE,
    WidgetBorderOption_ALWAYS_VISIBLE,
};

struct WidgetBorder
{
    enum WidgetBorderOption option;
    screenpos upLeft;
    vec2u16   size;

    utf16 *optTitle;
    enum ConsoleColorFG borderColor;
    enum ConsoleColorFG titleColor;
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
    enum WidgetId id;
    struct WidgetBorder border;
    struct WidgetCallbacks callbacks;

    enum WidgetTruncate truncateStatus;
};
