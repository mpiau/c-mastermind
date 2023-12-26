#include "widgets/widget_framerate.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "fps_counter.h"
#include "mouse.h"

#include "console.h"

struct WidgetFramerate
{
    struct Widget header;

    u64  lastAverageFPS;
    bool mouseHoveringWidget;
};


static void redraw_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_FRAMERATE );
    assert( widget->visibilityStatus != WidgetVisibilityStatus_HIDDEN );
    assert( widget->box.truncatedStatus == WidgetTruncatedStatus_NONE );

    struct WidgetFramerate *framerate = (struct WidgetFramerate *)widget;

    screenpos const contentUL = framerate->header.box.contentUpLeft;
 
    console_cursor_setpos( contentUL );
    console_color_fg( framerate->mouseHoveringWidget ? ConsoleColorFG_WHITE : ConsoleColorFG_BRIGHT_BLACK );
    console_draw( L"%3uFPS", framerate->lastAverageFPS );
    console_color_reset();
}


static void mouse_move_callback( struct Widget *widget, screenpos const /*old*/, screenpos const new )
{
    assert( widget->id == WidgetId_FRAMERATE );
    struct WidgetFramerate *framerate = (struct WidgetFramerate *)widget;

    screenpos const upLeft      = framerate->header.box.contentUpLeft;
    screenpos const bottomRight = framerate->header.box.contentBottomRight;

    bool const isMouseOnWidget =
        ( new.x >= upLeft.x && new.x <= bottomRight.x ) &&
        ( new.y >= upLeft.y && new.y <= bottomRight.y );

    if ( isMouseOnWidget != framerate->mouseHoveringWidget )
    {
        framerate->mouseHoveringWidget = isMouseOnWidget;
        widget->redrawNeeded = true;
    }
}


static void frame_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_FRAMERATE );
    struct WidgetFramerate *framerate = (struct WidgetFramerate *)widget;

    u64 const lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    if ( lastAverageFPS != framerate->lastAverageFPS )
    {
        framerate->lastAverageFPS = lastAverageFPS;
        widget->redrawNeeded = true;
    }
}


struct Widget *widget_framerate_create( void )
{
    struct WidgetFramerate *const framerate = malloc( sizeof( struct WidgetFramerate ) );
    if ( !framerate ) return NULL;

    struct Widget *const widget = &framerate->header;

    widget->id = WidgetId_FRAMERATE;
    widget->visibilityStatus = WidgetVisibilityStatus_VISIBLE;

    screenpos const borderUpLeft = (screenpos) { .x = 1, .y = 1 };
    vec2u16 const contentSize  = (vec2u16)   { .x = 6, .y = 1 };
    widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
    widget->box.borderOption = WidgetBorderOption_VISIBLE_ON_TRUNCATE;

    struct WidgetCallbacks *const callbacks = &framerate->header.callbacks;
    callbacks->frameCb = frame_callback;
    callbacks->mouseMoveCb = mouse_move_callback;
    callbacks->redrawCb = redraw_callback;

    // Specific to the widget 

    framerate->lastAverageFPS = 0;
    framerate->mouseHoveringWidget = false;

    return (struct Widget *)framerate;
}
