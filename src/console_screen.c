#include "console_screen.h"

#include "time_units.h"
#include "fps_counter.h"
#include "game.h"
#include "widgets/widget.h"
#include "widgets/widget_screen_size.h"

#include <windows.h>

enum // Constants
{
//    RESIZE_DELAY_NS = 200 * Time_MSEC_IN_NSEC,
    RESIZE_CALLBACKS_MAX_COUNT = 4
};

static HANDLE s_handle = INVALID_HANDLE_VALUE;
static vec2u16 s_incomingSize = {};
static vec2u16 s_currentSize = {};
// static nsec s_resizeTimestamp = 0;

static OnResizeCallback s_callbacks[RESIZE_CALLBACKS_MAX_COUNT] = {};
static u32 s_callbackCount = 0;


static vec2u16 get_console_size( HANDLE const handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( handle, &info ); // TODO check return value

    u16 const newScreenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    u16 const newscreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    return (vec2u16) { .x = newScreenWidth, .y = newscreenHeight };    
}


// ////////////////////////////////////////////////////////


bool console_screen_init( void const *handle )
{
    if ( handle == INVALID_HANDLE_VALUE ) return false;

    s_handle = (HANDLE)handle;
//    s_resizeTimestamp = 0;
    s_incomingSize = get_console_size( s_handle );
    return true;
}


void console_screen_frame( void )
{
    if ( s_incomingSize.x == s_currentSize.x && s_incomingSize.y == s_currentSize.y ) return;

  //  nsec const timestamp = time_get_timestamp_nsec();

    // Check if we had waited enough time to refresh the display
//    if ( timestamp - s_resizeTimestamp < RESIZE_DELAY_NS ) return; // C'est ici que l'on voudrait l'info pour notre widget

	vec2u16 const oldSize = s_currentSize;
    s_currentSize = s_incomingSize;

    for ( u32 idx; idx < s_callbackCount; ++idx )
    {
        s_callbacks[idx]( oldSize, s_currentSize );
    }
}


bool console_screen_resize( vec2u16 const newSize )
{
    // Handle possible invalid values ?
	if ( s_incomingSize.x == newSize.x && s_incomingSize.y == newSize.y ) return false;

    s_incomingSize = newSize;
    // s_resizeTimestamp = time_get_timestamp_nsec();

/*	struct Widget *widgetScreenSize = widget_optget( WidgetId_SCREEN_SIZE );
	if ( widgetScreenSize )
	{
		widget_screen_size_size_update( (struct Widget *)widgetScreenSize, newSize );
	}
*/
    return true;
}

/*
bool console_screen_is_being_resized( void )
{
	nsec const timestamp = time_get_timestamp_nsec();
	return timestamp - s_resizeTimestamp < RESIZE_DELAY_NS;
}*/


bool console_screen_is_too_small( void )
{
	return console_screen_is_width_too_small() || console_screen_is_height_too_small();
}


bool console_screen_is_width_too_small( void )
{
	return console_screen_get_size().w < GAME_SIZE_WIDTH;
}


bool console_screen_is_height_too_small( void )
{
	return console_screen_get_size().h < GAME_SIZE_HEIGHT;
}


bool console_screen_register_on_resize_callback( OnResizeCallback const callback )
{
    if ( callback == NULL )                              return false;
    if ( s_callbackCount == RESIZE_CALLBACKS_MAX_COUNT ) return false;

    s_callbacks[s_callbackCount] = callback;
    s_callbackCount++;
    return true;
}


vec2u16 console_screen_get_size( void )
{
    return s_currentSize;
}
