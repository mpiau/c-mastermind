#pragma once

#include "core_types.h"

enum WidgetType
{
    WidgetType_TIMER,
    WidgetType_COUNTDOWN,
    WidgetType_BOARD_SUMMARY,
    WidgetType_FRAMERATE_BAR,

    WidgetType_Count
};


typedef void (* HideCallback)( void );


struct Widget
{
    enum WidgetType type;
    int             id;

    // The higher the value, the latest is will be updated
    // You can think of the ZIndex in Web development, where the highest ZIndex is in front.
    u32             order;

    // If references > 0, don't free it because it's still referenced somewhere !
    // And if they shouldn't be referenced anymore, fix your code.
    // If references is < 0, also fix your damn code.
    int             references;

    HideCallback    hideCallback;
};
