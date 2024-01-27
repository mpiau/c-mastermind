#pragma once

#include "ui/ui.h"

struct Widget;

typedef void ( * WidgetEnableCb )( struct Widget *widget );
typedef void ( * WidgetDisableCb )( struct Widget *widget );
typedef void ( * WidgetFrameCb )( struct Widget *widget );

struct Widget
{
    char const * name;
    enum UIScene enabledScenes;

    WidgetEnableCb  enableCb;
    WidgetDisableCb disableCb;
    WidgetFrameCb   frameCb;
};


struct Widget *widget_framerate_create( void );
struct Widget *widget_screensize_create( void );
struct Widget *widget_mousepos_create( void );
struct Widget *widget_buildversion_create( void );
struct Widget *widget_bottom_nav_create( void );
struct Widget *widget_game_summary_create( void );
struct Widget *widget_timer_create( void );
struct Widget *widget_peg_selector_create( void );
struct Widget *widget_game_board_create( void );
