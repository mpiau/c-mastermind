#include "widgets/widget_fpsbar.h"

#include "widgets/widget.h"
#include "widgets/widget_definition.h"

#include "fps_counter.h"
#include "mouse.h"

#include "console.h"

enum Visibility
{
    Visibility_NONE         = 0x00,
    Visibility_FRAMERATE    = 0x01,
    Visibility_MILLISECONDS = 0x10,

    Visibility_ALL          = 0x11
};


struct WidgetFPSBar
{
    struct Widget header;

    enum Visibility visibility;
    u64             averageFPS;
    milliseconds    averageMsPerFrame;
    bool            mouseHoveringWidget;
};

static inline bool can_display_fps( struct WidgetFPSBar *fpsBar )
{
    return ( fpsBar->visibility & Visibility_FRAMERATE ) != 0;
}

static inline bool can_display_ms( struct WidgetFPSBar *fpsBar )
{
    return ( fpsBar->visibility & Visibility_MILLISECONDS ) != 0;
}


static void clear_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *fpsBar = (struct WidgetFPSBar *)widget;

    screenpos const upLeft = fpsBar->header.boxUpLeft;
    console_cursor_set_position( upLeft.y, upLeft.x );

    console_draw( L"%*lc", fpsBar->header.boxSize.x, L' ' );
}


static void draw_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *fpsBar = (struct WidgetFPSBar *)widget;

    // Avoid filling the buffer with only spaces at each draw call
    if ( fpsBar->visibility == Visibility_NONE ) return;

    screenpos const upLeft = fpsBar->header.boxUpLeft;
    console_cursor_set_position( upLeft.y, upLeft.x );
    console_color_fg( fpsBar->mouseHoveringWidget ? ConsoleColorFG_WHITE : ConsoleColorFG_BRIGHT_BLACK );

    // %2u -> assume we can't go over 99 fps. (capped at 90fps maximum)
    int charsDrawn = 0;
    
    if ( can_display_fps( fpsBar ) )
    {
        charsDrawn += console_draw( L"%2uFPS ", fpsBar->averageFPS );
    }

    if ( can_display_ms( fpsBar ) )
    {
        if ( fpsBar->averageMsPerFrame > 999 )
        {
            charsDrawn += console_draw( L"+" );
        }
        milliseconds const msToDraw = fpsBar->averageMsPerFrame > 999 ? 999 : fpsBar->averageMsPerFrame;
        charsDrawn += console_draw( L"%3llums", msToDraw );
    }

    // cleanup the rest of the widget space
    int const spaceLeft = fpsBar->header.boxSize.x - charsDrawn;
    assert( spaceLeft >= 0 ); // Otherwise, we would be outside the widget boundaries
    if ( spaceLeft > 0 )
    {
        console_draw( L"%*lc", spaceLeft, L' ' );
    }

    console_color_reset();
}


static void mouse_move_callback( struct Widget *widget, screenpos const /*old*/, screenpos const new )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *fpsBar = (struct WidgetFPSBar *)widget;

    screenpos const upLeft = fpsBar->header.boxUpLeft;
    screenpos const bottomRight = (screenpos) {
        .x = upLeft.x + fpsBar->header.boxSize.x - 1,
        .y = upLeft.y + fpsBar->header.boxSize.y - 1
    };
    bool const isMouseOnWidget =
        ( new.x >= upLeft.x && new.x <= bottomRight.x ) &&
        ( new.y >= upLeft.y && new.y <= bottomRight.y );

    if ( isMouseOnWidget != fpsBar->mouseHoveringWidget )
    {
        fpsBar->mouseHoveringWidget = isMouseOnWidget;
        draw_callback( widget );
    }
}


static void frame_callback( struct Widget *widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *fpsBar = (struct WidgetFPSBar *)widget;

    struct FPSCounter *fpsCounter = fpscounter_get_instance();
    u64 const averageFPS = fpscounter_average_framerate( fpsCounter );
    nanoseconds const averageFrameTimeNs = fpscounter_average_time( fpsCounter );
    milliseconds const averageMsPerFrame = nanoseconds_to_milliseconds( averageFrameTimeNs );

    if ( averageFPS == fpsBar->averageFPS && averageMsPerFrame == fpsBar->averageMsPerFrame )
    {
        // Nothing new to update, skip it to save a draw call.
        return;
    }

    fpsBar->averageFPS = averageFPS;
    fpsBar->averageMsPerFrame = averageMsPerFrame;

    // The FPSCounter has a buffer of 8 frames to fill before sending real average time.
    // Do we want to do something to prevent drawing 8 times not accurate fps at the beginning of the game ?
    // Seems like a very low task as it only impact the first 8 frames of the game start...
    draw_callback( widget );

    // Debug stuff
    // static u32 drawCall = 0;
    // drawCall += 1;
    // console_cursor_set_position( 10, 1 );
    // console_draw( L"FPS Widget draw calls: %u", drawCall );
}


struct Widget *widget_fpsbar_create( void )
{
    struct WidgetFPSBar *const fpsBar = malloc( sizeof( struct WidgetFPSBar ) );
    if ( !fpsBar ) return NULL;

    fpsBar->header.id = WidgetId_FPS_BAR;

    // Improve that part
    fpsBar->header.frameCallback = frame_callback;
    fpsBar->header.mouseMoveCallback = mouse_move_callback;

    // This one as well
    fpsBar->header.boxUpLeft = (screenpos) { .x = 2, .y = 2 };
    fpsBar->header.boxSize   = (vec2u16) { .x = 12, .y = 1 };

    fpsBar->visibility = Visibility_ALL;
    fpsBar->averageFPS = 0;
    fpsBar->averageMsPerFrame = 0;
    fpsBar->mouseHoveringWidget = false;

    return (struct Widget *)fpsBar;
}


// /////////////////////////////////////////////////////////////////////////////


void widget_fpsbar_show( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    if ( fpsBar->visibility != Visibility_ALL )
    {
        fpsBar->visibility = Visibility_ALL;
        draw_callback( widget );
    }
}


void widget_fpsbar_show_fps( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    if ( !can_display_fps( fpsBar ) )
    {
        fpsBar->visibility |= Visibility_FRAMERATE;
        draw_callback( widget );
    }
}


void widget_fpsbar_show_ms( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    if ( !can_display_ms( fpsBar ) )
    {
        fpsBar->visibility |= Visibility_MILLISECONDS;
        draw_callback( widget );
    }
}


void widget_fpsbar_hide( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    if ( fpsBar->visibility != Visibility_NONE )
    {
        fpsBar->visibility = Visibility_NONE;
        clear_callback( widget );
    }
}


void widget_fpsbar_hide_fps( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    if ( can_display_fps( fpsBar ) )
    {
        fpsBar->visibility &= ~Visibility_FRAMERATE;
        fpsBar->visibility == Visibility_NONE ? clear_callback( widget ) : draw_callback( widget );
    }
}


void widget_fpsbar_hide_ms( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    if ( can_display_ms( fpsBar ) )
    {
        fpsBar->visibility &= ~Visibility_MILLISECONDS;
        fpsBar->visibility == Visibility_NONE ? clear_callback( widget ) : draw_callback( widget );
    }
}


bool widget_is_shown( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    return fpsBar->visibility = Visibility_ALL;
}


bool widget_is_fps_shown( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    return can_display_fps( fpsBar );
}


bool widget_is_ms_shown( struct Widget *const widget )
{
    assert( widget->id == WidgetId_FPS_BAR );
    struct WidgetFPSBar *const fpsBar = (struct WidgetFPSBar *const)widget;

    return can_display_ms( fpsBar );
}
