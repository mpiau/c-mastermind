#include "console/console_screen.h"

#include "time_units.h"
#include "fps_counter.h"
#include "game.h"
#include "widgets/widget.h"

#include <windows.h>

enum // Constants
{
    RESIZE_CALLBACKS_MAX_COUNT = 4
};

// https://askubuntu.com/questions/528928/how-to-do-underline-bold-italic-strikethrough-color-background-and-size-i

struct CellAttr
{
    enum AttrColor color;
    enum AttrStyle style;
    enum AttrShade shade;
};

struct Cell
{
    utf16 character;
    struct CellAttr attributes;
};

struct ScreenCells
{
    union
    {
        struct Cell array[GAME_SIZE_HEIGHT][GAME_SIZE_HEIGHT];
        struct Cell raw[GAME_SIZE_HEIGHT * GAME_SIZE_HEIGHT];
    };
};

struct Screen
{
    void *handle;
    screenpos currPos;
    vec2u16 currSize;
    struct CellAttr currAttributes;
    struct ScreenCells currFrame;
    struct ScreenCells incomingFrame;
};


static void reset_attributes( struct CellAttr *const attr )
{
    attr->color = AttrColor_DEFAULT;
    attr->style = AttrStyle_DEFAULT;
    attr->shade = AttrShade_DEFAULT;
}

static void reset_frame_screen( struct ScreenCells *const frame )
{
    usize const cellsCount = ARR_COUNT( frame->raw );
    for ( usize idx = 0; idx < cellsCount; ++idx )
    {
        reset_attributes( &frame->raw[idx].attributes );
        frame->raw[idx].character = L' ';
    }
}

static vec2u16 get_screen_size( HANDLE const handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( handle, &info );

    u16 const newScreenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    u16 const newscreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    return (vec2u16) { .x = newScreenWidth, .y = newscreenHeight };    
}


struct Screen *console_screen_create( void *handle )
{
    struct Screen *screen = malloc( sizeof( *screen ) );
    if ( !screen ) return NULL;

    screen->handle = handle;
    screen->currPos = SCREENPOS( 1, 1 );
    screen->currSize = get_screen_size( handle );
    reset_attributes( &screen->currAttributes );
    reset_frame_screen( &screen->currFrame );
    memcpy( &screen->incomingFrame, &screen->currFrame, sizeof( &screen->incomingFrame ) );

    return screen;
}


void console_screen_destroy( struct Screen *screen )
{
    if ( screen )
    {
        free( screen );
    }
}


// 


void console_write( struct Screen *screen, utf16 const *format, ... )
{
    // Step One :
    // Write the user input into a local buffer and apply the printf format on it.
    // Once done, read the buffer and set each character + current attributes, and increment the position.
    // If the position is > than the screen size, stop.
}

void console_clear( struct Screen *screen )
{
    reset_frame_screen( &screen->incomingFrame );
}

void console_refresh_v2( struct Screen *screen )
{
    // We need to generate a buffer that it the difference between the current and the incoming (aka generating the virtual term sequences + buffer to write )
    // As we are only checking for the differences, we shouldn't have a flickering effect on the characters that haven't changed.
    // And it will avoid writing too much unnecessary chars for nothing in the console.
    // Then write this buffer into the console + set currentFrame == incomingFrame.
}


// 


void console_set_color( struct Screen *screen, enum AttrColor const color )
{
    screen->currAttributes.color = color;
}

void console_set_style( struct Screen *screen, enum AttrStyle const style )
{
    screen->currAttributes.style = style;
}

void console_set_shade( struct Screen *screen, enum AttrShade shade )
{
    screen->currAttributes.shade = shade;
}

bool console_set_attributes( struct Screen *screen, enum AttrColor const *optColor, enum AttrStyle const *optStyle, enum AttrShade const *optShade )
{
    if ( !screen ) return false;

    if ( optColor ) console_set_color( screen, *optColor );
    if ( optStyle ) console_set_style( screen, *optStyle );
    if ( optShade ) console_set_shade( screen, *optShade );

    return true;    
}


enum AttrColor console_color( struct Screen const *screen )
{
    return screen->currAttributes.color;
}

enum AttrStyle console_style( struct Screen const *screen )
{
    return screen->currAttributes.style;
}

enum AttrShade console_shade( struct Screen const *screen )
{
    return screen->currAttributes.shade;
}

bool console_attributes( struct Screen const *screen, enum AttrColor *outOptColor, enum AttrStyle *outOptStyle, enum AttrShade *outOptShade )
{
    if ( !screen ) return false;

    if ( outOptColor ) *outOptColor = console_color( screen );
    if ( outOptStyle ) *outOptStyle = console_style( screen );
    if ( outOptShade ) *outOptShade = console_shade( screen );

    return true;
}


void console_set_pos( struct Screen *screen, screenpos const pos )
{
    screen->currPos = pos;
}

screenpos console_pos( struct Screen const *screen )
{
    return screen->currPos;
}


static HANDLE s_handle = INVALID_HANDLE_VALUE;
static vec2u16 s_incomingSize = {};
static vec2u16 s_currentSize = {};
// static nsec s_resizeTimestamp = 0;

static OnScreenResizeCallback s_callbacks[RESIZE_CALLBACKS_MAX_COUNT] = {};
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

/*	struct Widget *widgetScreenSize = widget_optget( ComponentId_SCREEN_SIZE );
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


bool console_screen_register_on_resize_callback( OnScreenResizeCallback const callback )
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
