#include "console_screen.h"

#include "time_units.h"
#include "fps_counter.h"

#include <windows.h>

enum // Constants
{
    RESIZE_DELAY_NS = 100 * 1000 * 1000, // 100 nanoseconds
    RESIZE_CALLBACKS_MAX_COUNT = 16
};

static HANDLE s_handle = INVALID_HANDLE_VALUE;
static vec2u16 s_newSize = {};
static vec2u16 s_oldSize = {}; // Not equal to newSize to trigger a first draw at the beginning.
nanoseconds s_resizeTimestamp = 0;

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
    s_resizeTimestamp = 0;
    s_newSize = get_console_size( s_handle );
    return true;
}


void console_screen_frame( void )
{
    if ( s_newSize.x == s_oldSize.x && s_newSize.y == s_oldSize.y ) return;

    nanoseconds const timestamp = get_timestamp_nanoseconds();

    // Check if we had waited enough time to refresh the display
    if ( timestamp - s_resizeTimestamp < RESIZE_DELAY_NS ) return;

    for ( u32 idx; idx < s_callbackCount; ++idx )
    {
        s_callbacks[idx]( s_oldSize, s_newSize );
    }
    s_oldSize = s_newSize;
}


bool console_screen_resize( vec2u16 const newSize )
{
    // Handle possible invalid values ?

    s_newSize = newSize;
    s_resizeTimestamp = get_timestamp_nanoseconds();
    return true;
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
    return s_newSize;
}
