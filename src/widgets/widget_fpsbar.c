#include "widgets/widget_fpsbar.h"

#include "widgets/widgets.h"
#include "widgets/widget_definition.h"

#include "fps_counter.h"

#include "console.h"

struct WidgetFPSBar
{
    struct Widget header; // or base ?
    // struct WidgetStyle style ?

    // Specific stuff
    struct FPSCounter *fpsCounter;

    u64 averageFPS;
    milliseconds averageMsPerFrame;
};


struct WidgetFPSBar s_widgetFPSBar = {};
static bool s_init = false;


static void hide_callback( void )
{
    // Do something
}


// TODO On resize, if the size of the screen is < to what's needed, don't draw anything
// Until rechecking on resize.

static void draw( void )
{
    // TODO It's hardcoded here, it should be relative to widget position.
    screenpos upLeft = s_widgetFPSBar.header.boxUpLeft;
	console_cursor_set_position( upLeft.y, upLeft.x );
	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

    // %2u -> assume we can't go over 99 fps. (capped at 90fps maximum)
 	int charsDrawn = console_draw( L"%2uFPS ", s_widgetFPSBar.averageFPS );

	if ( s_widgetFPSBar.averageMsPerFrame > 999 )
    {
        charsDrawn += console_draw( L"+" );
    }
    milliseconds const msToDraw = s_widgetFPSBar.averageMsPerFrame > 999 ? 999 : s_widgetFPSBar.averageMsPerFrame;
	charsDrawn += console_draw( L"%3llums", msToDraw ); // spaces at the end to remove size fluctuation if bigger size before

    // cleanup the rest of the widget space
    int const spaceLeft = s_widgetFPSBar.header.boxSize.x - charsDrawn;
    assert( spaceLeft >= 0 );
    if ( spaceLeft > 0 )
    {
        console_draw( L"%*lc", spaceLeft, L' ' );
    }
}


static void frame_callback( void )
{
    assert( s_init );

    u64 const averageFPS = fpscounter_average_framerate( s_widgetFPSBar.fpsCounter );
	nanoseconds const averageFrameTimeNs = fpscounter_average_time( s_widgetFPSBar.fpsCounter );
    milliseconds const averageMsPerFrame = nanoseconds_to_milliseconds( averageFrameTimeNs );

    if ( averageFPS == s_widgetFPSBar.averageFPS && averageMsPerFrame == s_widgetFPSBar.averageMsPerFrame )
    {
        // Nothing new to update, skip it to save a draw call.
        return;
    }

    s_widgetFPSBar.averageFPS = averageFPS;
    s_widgetFPSBar.averageMsPerFrame = averageMsPerFrame;

    // The FPSCounter has a buffer of 8 frames to fill before sending real average time.
    // Do we want to do something to prevent drawing 8 times not accurate fps at the beginning of the game ?
    // Seems like a very low task as it only impact the first 8 frames of the game start...

    draw();

    // Debug stuff
    // static u32 drawCall = 0;
    // drawCall += 1;
    // console_cursor_set_position( 10, 1 );
    // console_draw( L"FPS Widget draw calls: %u", drawCall );
}


bool widget_fpsbar_init( struct FPSCounter *fpsCounter )
{
    if ( s_init ) { return false; }
    if ( !fpsCounter ) { return false; }

    // Avoid reinit if already init
    s_widgetFPSBar.header.id = WidgetId_FPS_BAR;
    s_widgetFPSBar.header.references = 0;
    s_widgetFPSBar.header.hideCallback = hide_callback;
    s_widgetFPSBar.header.frameCallback = frame_callback;

    s_widgetFPSBar.header.boxUpLeft = (screenpos) { .x = 1, .y = 1 };
    s_widgetFPSBar.header.boxSize   = (vec2u16) { .x = 12, .y = 1 };

    s_widgetFPSBar.fpsCounter = fpsCounter;
    s_widgetFPSBar.averageFPS = 0;
    s_widgetFPSBar.averageMsPerFrame = 0;

    // Define size of the widget
    // Define style of the widget

    widgets_hook( (struct Widget *)&s_widgetFPSBar );

    s_init = true;
    return s_init;
}


void widget_fpsbar_uninit( void )
{
    widgets_unhook( (struct Widget const *)&s_widgetFPSBar );
    s_init = false;
}
