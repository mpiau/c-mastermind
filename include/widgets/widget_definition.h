#pragma once

#include "core_types.h"
#include "core_unions.h"

enum WidgetId
{
    WidgetId_TIMER,
    WidgetId_COUNTDOWN,
    WidgetId_BOARD_SUMMARY,
    WidgetId_FPS_BAR,

    WidgetId_Count,
};


typedef void (* HideCallback)( void );
typedef void (* FrameCallback)( void );


struct Widget
{
    enum WidgetId id;

    // If references > 0, don't free it because it's still referenced somewhere !
    // And if they shouldn't be referenced anymore, fix your code.
    // If references is < 0, also fix your damn code.
    int             references;

    HideCallback    hideCallback;
    FrameCallback   frameCallback;

    screenpos boxUpLeft;
    vec2u16   boxSize;
};
