#include "console/console_screen.h"

#include "time_units.h"
#include "fps_counter.h"
#include "game.h"
#include "widgets/widget.h"

#include <stdio.h>
#include <windows.h>

enum // Constants
{
    RESIZE_CALLBACKS_MAX_COUNT = 4,

    DEFAULT_CHARACTER = L' '
};

// https://askubuntu.com/questions/528928/how-to-do-underline-bold-italic-strikethrough-color-background-and-size-i


struct Cell
{
    utf16 character;
    struct Style style;
};

struct Screen
{
    union
    {
        struct Cell array[GAME_SIZE_HEIGHT][GAME_SIZE_WIDTH];
        struct Cell raw[GAME_SIZE_HEIGHT * GAME_SIZE_WIDTH];
    };
};

struct ScreenData
{
    void const *handle;
    screenpos currPos;
    vec2u16 currSize;
    struct Style currStyle;
    struct Screen currFrame;
    struct Screen incomingFrame;
};

static struct ScreenData s_screenData = {};


static inline struct Style default_style( void )
{
    return (struct Style) { .color = Color_DEFAULT, .dispAttr = DispAttr_NONE };
}


static inline struct Cell *get_incoming_frame_cell( usize const y, usize const x )
{
    return &s_screenData.incomingFrame.array[y][x];
}


static inline struct Cell *get_incoming_frame_current_cell( void )
{
    return get_incoming_frame_cell( s_screenData.currPos.y - 1, s_screenData.currPos.x - 1 );
}


static void reset_screen( struct Screen *const screen )
{
    usize const cellsCount = ARR_COUNT( screen->raw );
    for ( usize idx = 0; idx < cellsCount; ++idx )
    {
        screen->raw[idx].style = default_style();
        screen->raw[idx].character = DEFAULT_CHARACTER;
    }
}


static bool cell_equals( struct Cell const *lhs, struct Cell const *rhs )
{
    return memcmp( lhs, rhs, sizeof( *lhs ) ) == 0;
}


static bool style_equals( struct Style const lhs, struct Style const rhs )
{
    return memcmp( &lhs, &rhs, sizeof( lhs ) ) == 0;
}


static vec2u16 get_screen_size( void const *handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( (HANDLE)handle, &info );

    u16 const newScreenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    u16 const newscreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    return VEC2U16( newScreenWidth, newscreenHeight );
}


bool console_screen_init( void const *handle )
{
    s_screenData.handle = handle;
    s_screenData.currPos = SCREENPOS( 1, 1 );
    s_screenData.currSize = get_screen_size( handle );
    s_screenData.currStyle = default_style();
    reset_screen( &s_screenData.currFrame );
    memcpy( &s_screenData.incomingFrame, &s_screenData.currFrame, sizeof( s_screenData.incomingFrame ) );

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

    struct Style const style = s_screenData.currStyle;
    for ( usize idx = 0; idx < bufferSize && s_screenData.currPos.x <= s_screenData.currSize.w; ++idx )
    {
        struct Cell *cell = get_incoming_frame_current_cell();
        cell->character = buffer[idx];
        cell->style = style;
        s_screenData.currPos.x += 1;
    }

    return bufferSize;
}


void console_clear( void )
{
    reset_screen( &s_screenData.incomingFrame );
}


// 


struct Style style_make( enum Color const color, enum ColorBrightness const brightness, enum DispAttr const attr )
{
    return (struct Style) {
        .color = color | brightness,
        .dispAttr = attr
    };
}


void console_set_style( struct Style const style )
{
    s_screenData.currStyle = style;
}


static inline bool style_is_bold( struct Style const style )
{
    return ( style.dispAttr & DispAttr_BOLD ) == DispAttr_BOLD;
}

static inline bool style_is_faint( struct Style const style )
{
    return ( style.dispAttr & DispAttr_FAINT ) == DispAttr_FAINT;
}

static inline bool style_is_italic( struct Style const style )
{
    return ( style.dispAttr & DispAttr_ITALIC ) == DispAttr_ITALIC;
}

static inline bool style_is_underline( struct Style const style )
{
    return ( style.dispAttr & DispAttr_UNDERLINE ) == DispAttr_UNDERLINE;
}


static u8 get_fgcolor_code( struct Style const style )
{
    bool const isBrightForeground = ( ( style.color & ColorBrightness_FG ) != 0 );
    u8 const brightParam = isBrightForeground ? 60 : 0;

    static_assert( ColorFG_MaskAll == 0b00111000, "If the Mask has changed, you need to change this conversion as well" );
    u8 const colorCode = ( style.color & ColorFG_MaskAll ) >> 3;

    return 30 + colorCode + brightParam;
}


static u8 get_bgcolor_code( struct Style const style )
{
    bool const isBrightForeground = ( ( style.color & ColorBrightness_BG ) != 0 );
    u8 const brightParam = isBrightForeground ? 60 : 0;

    static_assert( ColorBG_MaskAll == 0b00000111, "If the Mask has changed, you need to change this conversion as well" );
    u8 const colorCode = ( style.color & ColorBG_MaskAll );

    return 40 + colorCode + brightParam;    
}


static int generate_color_sequence( struct Style const style, utf16 *const buffer, usize const bufferSize )
{
    return swprintf( buffer, bufferSize, L"\x1b[%u;%um", get_fgcolor_code( style ), get_bgcolor_code( style ) );
}


static int generate_attributes_sequence( struct Style const style, utf16 *const buffer, usize const bufferSize )
{
    // Start with a 0 to reset the old attributes first.
    int nbWritten = swprintf( buffer, bufferSize, L"\x1b[0" );

    if ( style_is_bold( style ) )  { nbWritten += swprintf( buffer + nbWritten, bufferSize - nbWritten, L";1" ); }
    if ( style_is_faint( style ) ) { nbWritten += swprintf( buffer + nbWritten, bufferSize - nbWritten, L";2" ); }
    // TODO : Complete the different edge cases [...]

    nbWritten += swprintf( buffer + nbWritten, bufferSize - nbWritten, L"m" );

    return nbWritten;
}


int generate_style_sequence( struct Style const style, utf16 *const buffer, usize const bufferSize )
{
    int nbWritten = 0;
    nbWritten += generate_attributes_sequence( style, buffer + nbWritten, bufferSize - nbWritten );
    nbWritten += generate_color_sequence( style, buffer + nbWritten, bufferSize - nbWritten );
    return nbWritten;
}


void console_refresh_v2( void )
{
    static utf16 refreshBuffer[8092] = {};
    usize bufPos = 0;

    // default State
    struct Style style = default_style();
    screenpos cursorPos = SCREENPOS( 1, 1 );

    for ( usize height = 0; height < s_screenData.currSize.h && height < GAME_SIZE_HEIGHT; ++height )
    {
        for ( usize width = 0; width < s_screenData.currSize.w && width < GAME_SIZE_WIDTH; ++width )
        {
            struct Cell *currentCell = &s_screenData.currFrame.array[height][width];
            struct Cell const *incomingCell = &s_screenData.incomingFrame.array[height][width];
            if ( cell_equals( currentCell, incomingCell ) ) continue;

            screenpos newpos = SCREENPOS( width + 1, height + 1 );
            if ( newpos.x != cursorPos.x || newpos.y != cursorPos.y )
            {
                bufPos += snwprintf( refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos, L"\x1B[%u;%uH", newpos.y, newpos.x );
                cursorPos = newpos;
            }

            if ( !style_equals( currentCell->style, incomingCell->style ) || !style_equals( incomingCell->style, style ) )
            {
                bufPos += generate_style_sequence( incomingCell->style, refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos );
                style = incomingCell->style;
                currentCell->style = incomingCell->style;
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



// [...]


void console_set_pos( screenpos const pos )
{
    s_screenData.currPos = pos;
}

screenpos console_pos( void )
{
    return s_screenData.currPos;
}


static OnScreenResizeCallback s_callbacks[RESIZE_CALLBACKS_MAX_COUNT] = {};
static u32 s_callbackCount = 0;

void console_on_screen_resize( vec2u16 const new )
{
    vec2u16 const old = s_screenData.currSize;
    if ( old.x == new.x && old.y == new.y ) return;

    if ( old.x < new.x && old.x <= GAME_SIZE_WIDTH )
    {
        // draw the missing columns, erased when the window has been reduced.
    }

    if ( old.y < new.y && old.y <= GAME_SIZE_HEIGHT )
    {
        // draw the missing lines, erased when console size has been reduced.
    }

    s_screenData.currSize = new;

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
    return s_screenData.currSize;
//    return s_currentSize;
}
