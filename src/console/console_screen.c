#include "console/console_screen.h"

#include "time_units.h"
#include "fps_counter.h"
#include "game.h"
#include "widgets/widget.h"

#include <stdio.h>
#include <windows.h>

enum // Constants
{
    RESIZE_CALLBACKS_MAX_COUNT = 4
};

// https://askubuntu.com/questions/528928/how-to-do-underline-bold-italic-strikethrough-color-background-and-size-i

struct Cell
{
    utf16 character;
    struct Attr attributes;
};

struct ScreenCells
{
    union
    {
        struct Cell array[GAME_SIZE_HEIGHT][GAME_SIZE_WIDTH];
        struct Cell raw[GAME_SIZE_HEIGHT * GAME_SIZE_WIDTH];
    };
};

struct Screen
{
    void const *handle;
    screenpos currPos;
    vec2u16 currSize;
    struct Attr currAttributes;
    struct ScreenCells currFrame;
    struct ScreenCells incomingFrame;
};

static struct Screen s_screen = {};


static void reset_attributes( struct Attr *const attr )
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

static bool cell_equals( struct Cell const *lhs, struct Cell const *rhs )
{
    return memcmp( lhs, rhs, sizeof( *lhs ) ) == 0;
}

static bool attr_equals( struct Attr const *lhs, struct Attr const *rhs )
{
    return memcmp( lhs, rhs, sizeof( *lhs ) ) == 0;
}

static vec2u16 get_screen_size( void const *handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( (HANDLE)handle, &info );

    u16 const newScreenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    u16 const newscreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    return (vec2u16) { .x = newScreenWidth, .y = newscreenHeight };    
}


bool console_screen_init( void const *handle )
{
    s_screen.handle = handle;
    s_screen.currPos = SCREENPOS( 1, 1 );
    s_screen.currSize = get_screen_size( handle );
    reset_attributes( &s_screen.currAttributes );
    reset_frame_screen( &s_screen.currFrame );
    memcpy( &s_screen.incomingFrame, &s_screen.currFrame, sizeof( s_screen.incomingFrame ) );

    return true;
}


void console_screen_uninit( void )
{
    //
}


// 


int console_write( utf16 const *format, ... )
{
    static utf16 buffer[512];

    va_list args;
	va_start( args, format );
    int const bufferSize = vsnwprintf( buffer, ARR_COUNT( buffer ), format, args );
	va_end( args );

    struct Attr const *attributes = &s_screen.currAttributes;
    for ( usize idx = 0; idx < bufferSize && s_screen.currPos.x <= s_screen.currSize.w; ++idx )
    {
        struct Cell *cell = &s_screen.incomingFrame.array[s_screen.currPos.y - 1][s_screen.currPos.x - 1];
        cell->character = buffer[idx];
        cell->attributes = *attributes;
        s_screen.currPos.x += 1;
    }

    return bufferSize;
}

void console_clear( void )
{
    reset_frame_screen( &s_screen.incomingFrame );
}

void generate_attr_style_sequence( enum AttrStyle style, utf16 *buffer, usize bufSize, usize *outBufPos, bool *first )
{
    if ( ( style & AttrStyle_BOLD ) != 0 )
    {
        *outBufPos += snwprintf( buffer + *outBufPos, bufSize - *outBufPos, *first ? L"1" : L";1" );
        *first = false;
    }
    // [...]
}

void generate_attr_shade_sequence( enum AttrShade shade, utf16 *buffer, usize bufSize, usize *outBufPos, bool *first )
{
    if ( ( shade & AttrShade_DARK ) != 0 )
    {
        *outBufPos += snwprintf( buffer + *outBufPos, bufSize - *outBufPos, *first ? L"2" : L";2" );
        *first = false;
    }
}

usize get_foreground_color( enum AttrColor color )
{
    enum AttrColor fgColor = ( color & AttrColor_MaskForeground );

    switch ( fgColor )
    {
        case 0: return 39; // default;
        case AttrColor_BLACK_FG: return 30;
        case AttrColor_RED_FG: return 31;
        case AttrColor_GREEN_FG: return 32;
        case AttrColor_YELLOW_FG: return 33;
        case AttrColor_BLUE_FG: return 34;
        case AttrColor_MAGENTA_FG: return 35;
        case AttrColor_CYAN_FG: return 36;
        case AttrColor_WHITE_FG: return 37;
        default: assert( false );
    }
}

usize get_background_color( enum AttrColor color )
{
    enum AttrColor bgColor = ( color & AttrColor_MaskBackground );

    switch ( bgColor )
    {
        case 0: return 49; // default;
        case AttrColor_BLACK_BG: return 40;
        case AttrColor_RED_BG: return 41;
        case AttrColor_GREEN_BG: return 42;
        case AttrColor_YELLOW_BG: return 43;
        case AttrColor_BLUE_BG: return 44;
        case AttrColor_MAGENTA_BG: return 45;
        case AttrColor_CYAN_BG: return 46;
        case AttrColor_WHITE_BG: return 47;
        default: assert( false );
    }
}

void generate_attr_color_sequence( enum AttrColor color, bool isBright, utf16 *buffer, usize bufSize, usize *outBufPos, bool *first )
{
    bool const hasBgColor = ( color & AttrColor_MaskBackground ) != 0;
    if ( hasBgColor )
    {
        usize colorNb = get_background_color( color );
        if ( isBright && colorNb != 49 ) colorNb += 60;
        *outBufPos += snwprintf( buffer + *outBufPos, bufSize - *outBufPos, *first ? L"%u" : L";%u", colorNb );
        *first = false;
    }

    bool const hasFgColor = ( color & AttrColor_MaskForeground ) != 0;
    if ( hasFgColor )
    {
        usize colorNb = get_foreground_color( color );
        if ( isBright && colorNb != 39 ) colorNb += 60;
        *outBufPos += snwprintf( buffer + *outBufPos, bufSize - *outBufPos, *first ? L"%u" : L";%u", colorNb );
        *first = false;
    }
}


void generate_attr_virtual_sequence( struct Attr const *attr, utf16 *buffer, usize bufferSize, usize *outBufPos )
{
    // It works, but clean this mess

    *outBufPos += snwprintf( buffer + *outBufPos, bufferSize - *outBufPos, L"\x1b[0;0m" );

    if ( attr->color == AttrColor_DEFAULT && attr->style == AttrStyle_DEFAULT && attr->shade == AttrShade_DEFAULT )
    {
        // Nothing specific to write here, just send the reset.
        return;
    }

    *outBufPos += snwprintf( buffer + *outBufPos, bufferSize - *outBufPos, L"\x1b[" );
    bool first = true;

    if ( attr->style != AttrStyle_DEFAULT )
    {
        generate_attr_style_sequence( attr->style, buffer, bufferSize, outBufPos, &first );
    }
    if ( attr->shade != AttrShade_DEFAULT )
    {
        generate_attr_shade_sequence( attr->shade, buffer, bufferSize, outBufPos, &first );
    }
    // For the colours, the shade BRIGHT will modify their values, so it's important to check for it
    if ( attr->color != AttrShade_DEFAULT || attr->shade == AttrShade_BRIGHT )
    {
        generate_attr_color_sequence( attr->color, attr->shade == AttrShade_BRIGHT, buffer, bufferSize, outBufPos, &first );
    }

    *outBufPos += snwprintf( buffer + *outBufPos, bufferSize - *outBufPos, L"m" );
}


void console_refresh_v2( void )
{
    static utf16 refreshBuffer[8092] = {};
    usize bufPos = 0;

    struct Attr attributes;
    reset_attributes( &attributes );

    screenpos cursorPos = SCREENPOS( 1, 1 );

    for ( usize height = 0; height < s_screen.currSize.h && height < GAME_SIZE_HEIGHT; ++height )
    {
        for ( usize width = 0; width < s_screen.currSize.w && width < GAME_SIZE_WIDTH; ++width )
        {
            struct Cell *currentCell = &s_screen.currFrame.array[height][width];
            struct Cell const *incomingCell = &s_screen.incomingFrame.array[height][width];
            if ( cell_equals( currentCell, incomingCell ) ) continue;

            screenpos newpos = SCREENPOS( width + 1, height + 1 );
            if ( newpos.x != cursorPos.x || newpos.y != cursorPos.y )
            {
                bufPos += snwprintf( refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos, L"\x1B[%u;%uH", newpos.y, newpos.x );
                cursorPos = newpos;
            }

            // TODO: This condition has a problem, because without it the attributes are correct on screen.
            // However it adds a bigger load on the output so we need to find the problem and fix it.
            if ( !attr_equals( &currentCell->attributes, &incomingCell->attributes ) && !attr_equals( &incomingCell->attributes, &attributes ) )
            {
                // The attributes are different between the two, and our current attributes in the refresh buffer is not what we want.
                // So we need to update that with virtual key sequences.
                generate_attr_virtual_sequence( &incomingCell->attributes, refreshBuffer, ARR_COUNT( refreshBuffer ), &bufPos );

                attributes = incomingCell->attributes;
                currentCell->attributes = incomingCell->attributes;
            }

            bufPos += snwprintf( refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos, L"%lc", incomingCell->character );
            cursorPos.x += 1;
            *currentCell = *incomingCell;
        }
    }

    if ( bufPos > 0 )
    {
        // Set the cursor back to 1;1 in order to assume that for each refresh, we are starting with the cursor at this position on the screen
        snwprintf( refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos, L"\x1B[1;1H" );
        wprintf( refreshBuffer );
    }
}


// 


void console_set_color( enum AttrColor const color )
{
    s_screen.currAttributes.color = color;
}

void console_set_style( enum AttrStyle const style )
{
    s_screen.currAttributes.style = style;
}

void console_set_shade( enum AttrShade shade )
{
    s_screen.currAttributes.shade = shade;
}

void console_set_attr( struct Attr const attributes )
{
    s_screen.currAttributes = attributes;
}

void console_reset_attr( void )
{
    reset_attributes( &s_screen.currAttributes );
}

enum AttrColor console_color( void )
{
    return s_screen.currAttributes.color;
}

enum AttrStyle console_style( void )
{
    return s_screen.currAttributes.style;
}

enum AttrShade console_shade( void )
{
    return s_screen.currAttributes.shade;
}

struct Attr console_attr( void )
{
    return s_screen.currAttributes;
}


void console_set_cpos( screenpos const pos )
{
    s_screen.currPos = pos;
}

screenpos console_cpos( void )
{
    return s_screen.currPos;
}


static OnScreenResizeCallback s_callbacks[RESIZE_CALLBACKS_MAX_COUNT] = {};
static u32 s_callbackCount = 0;

void console_on_screen_resize( vec2u16 const new )
{
    vec2u16 const old = s_screen.currSize;
    if ( old.x == new.x && old.y == new.y ) return;

    if ( old.x < new.x && old.x <= GAME_SIZE_WIDTH )
    {
        // draw the missing columns, erased when the window has been reduced.
    }

    if ( old.y < new.y && old.y <= GAME_SIZE_HEIGHT )
    {
        // draw the missing lines, erased when console size has been reduced.
    }

    s_screen.currSize = new;

    // TODO execute callbacks, or call components_on_resize() directly.

    // TMP, just for to make resize component work again
    for ( u32 idx = 0; idx < s_callbackCount; ++idx )
    {
        s_callbacks[idx]( old, new );
    }
}







static HANDLE s_handle = INVALID_HANDLE_VALUE;
static vec2u16 s_incomingSize = {};
static vec2u16 s_currentSize = {};
// static nsec s_resizeTimestamp = 0;

static vec2u16 get_console_size( HANDLE const handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( handle, &info ); // TODO check return value

    u16 const newScreenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    u16 const newscreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    return (vec2u16) { .x = newScreenWidth, .y = newscreenHeight };    
}


// ////////////////////////////////////////////////////////


/*bool console_screen_init( void const *handle )
{
    if ( handle == INVALID_HANDLE_VALUE ) return false;

    s_handle = (HANDLE)handle;
//    s_resizeTimestamp = 0;
    s_incomingSize = get_console_size( s_handle );
    return true;
}*/


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
//	if ( s_incomingSize.x == newSize.x && s_incomingSize.y == newSize.y ) return false;

  //  s_incomingSize = newSize;
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
    return s_screen.currSize;
//    return s_currentSize;
}
