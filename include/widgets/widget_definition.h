#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "widgets/widget.h"
#include "mouse.h"

typedef void ( *WidgetFrameCallback )( struct Widget *widget );
typedef void ( *WidgetMouseMoveCallback )( struct Widget *widget, screenpos oldPos, screenpos newPos );

enum WidgetTruncate
{
    WidgetTruncate_NONE   = 0x00,
    WidgetTruncate_X_AXIS = 0x01,
    WidgetTruncate_Y_AXIS = 0x10
};

struct Widget
{
    enum WidgetId id;
    enum WidgetTruncate truncateStatus;

    // If references > 0, don't free it because it's still referenced somewhere !
    // And if they shouldn't be referenced anymore, fix your code.
    // If references is < 0, also fix your damn code.
    int             references;

    WidgetFrameCallback   		frameCallback;
	WidgetMouseMoveCallback 		mouseMoveCallback;

    screenpos boxUpLeft;
    vec2u16   boxSize;
};


struct WidgetBorder
{
    screenpos upLeft;
    vec2u16   size;

    utf16 *optTitle;

    // ConsoleColor borderBackground;
    // ConsoleColor borderForeground;
    // bool showBorders;
    // bool hasBorders;
    // bool displayTitle
    // utf16 *title
};

struct WidgetCallbacks
{
    WidgetFrameCallback   		frameCallback;
  	WidgetMouseMoveCallback 		mouseMoveCallback;
    // [...] 
};