#include "ui/ui.h"
#include "ui/widgets.h"

#include <stdlib.h>


enum WidgetId
{
    WidgetId_FRAMERATE,
    WidgetId_SCREENSIZE,
    WidgetId_MOUSEPOS,
    WidgetId_BUILDVERSION,
    WidgetId_BOTTOM_NAV,
    WidgetId_GAME_SUMMARY,
    WidgetId_TIMER,
    WidgetId_PEG_SELECTOR,
    WidgetId_GAME_BOARD,
    WidgetId_PEG_TRACKING,

    WidgetId_Count
};


typedef struct Widget * ( * WidgetCreateFunc )( void );

static struct Widget *s_widgets[WidgetId_Count];
static bool s_widgetsEnabled[WidgetId_Count];
static enum UIScene s_currScene;


static void init_widget( enum WidgetId const id, WidgetCreateFunc const createFunction )
{
    struct Widget *widget = createFunction();
    if ( !widget ) return;

    s_widgets[id] = widget;
    s_widgetsEnabled[id] = false;
}


bool ui_init( void )
{
    s_currScene = UIScene_NONE;

    init_widget( WidgetId_FRAMERATE, widget_framerate_create );
    init_widget( WidgetId_SCREENSIZE, widget_screensize_create );
    init_widget( WidgetId_MOUSEPOS, widget_mousepos_create );
    init_widget( WidgetId_BUILDVERSION, widget_buildversion_create );
    init_widget( WidgetId_BOTTOM_NAV, widget_bottom_nav_create );
    init_widget( WidgetId_GAME_SUMMARY, widget_game_summary_create );
    init_widget( WidgetId_TIMER, widget_timer_create );
    init_widget( WidgetId_PEG_SELECTOR, widget_peg_selector_create );
    init_widget( WidgetId_GAME_BOARD, widget_game_board_create );
    init_widget( WidgetId_PEG_TRACKING, widget_peg_tracking_create );

    return true;
}


void ui_uninit( void )
{
    for ( usize idx = 0; idx < WidgetId_Count; ++idx )
    {
        if ( s_widgets[idx] )
        {
            free( s_widgets[idx] );
            s_widgets[idx] = NULL;
        }
    }
}


void ui_frame( void )
{
    for ( usize idx = 0; idx < WidgetId_Count; ++idx )
    {
        struct Widget *widget = s_widgets[idx];
        if ( widget && s_widgetsEnabled[idx] && widget->frameCb )
        {
            widget->frameCb( widget );
        }
    }
}


bool ui_change_scene( enum UIScene const scene )
{
    if ( s_currScene == scene ) return false;

    s_currScene = scene;

    for ( usize idx = 0; idx < WidgetId_Count; ++idx )
    {
        struct Widget *widget = s_widgets[idx];
        if ( !widget ) continue;

        if ( !( widget->enabledScenes & scene ) && s_widgetsEnabled[idx] )
        {
            if ( widget->disableCb )
            {
                widget->disableCb( widget );
            }
            s_widgetsEnabled[idx] = false;
        }
    }

    for ( usize idx = 0; idx < WidgetId_Count; ++idx )
    {
        struct Widget *widget = s_widgets[idx];
        if ( !widget ) continue;

        if ( ( widget->enabledScenes & scene ) && !s_widgetsEnabled[idx] )
        {
            if ( widget->enableCb )
            {
                widget->enableCb( widget );
            }
            s_widgetsEnabled[idx] = true;
        }
    }

    return true;
}
