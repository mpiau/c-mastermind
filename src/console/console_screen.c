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

    DEFAULT_UNICODE = L' ',
    BRIGHT_VALUE = 60, // Value added on a color to make it bright
    FOREGROUND_COLOR_BASE = 30,
    BACKGROUND_COLOR_BASE = 40,
};

// https://askubuntu.com/questions/528928/how-to-do-underline-bold-italic-strikethrough-color-background-and-size-i


struct Character
{
    utf16 unicode;
    struct Properties properties;
};

union Screen
{
    struct Character charsArray[GAME_SIZE_HEIGHT][GAME_SIZE_WIDTH];
    struct Character charsRaw[GAME_SIZE_AREA];
};

struct ScreenData
{
    void const *handle;

    union screensize gameScreenSize;
    vec2u16          realScreenSize;

    union screenpos   currPos;
    struct Properties currProperties;

    union Screen screen;
};

static struct ScreenData s_screenData = {};


static inline struct Properties default_properties( void )
{
    return (struct Properties) { .color = Color_DEFAULT, .flags = AttrFlags_NONE };
}

static inline struct Character default_character( void )
{
    return (struct Character) { .unicode = DEFAULT_UNICODE, .properties = default_properties() };
}

static bool properties_equals( struct Properties lhs, struct Properties rhs )
{
    // Skip the special flags here
    return lhs.color == rhs.color && ( lhs.flags & AttrFlags_MaskAll ) == ( rhs.flags & AttrFlags_MaskAll );
}


static inline struct Character *get_character_at_pos( union screenpos const pos )
{
    return &s_screenData.screen.charsArray[pos.y - 1][pos.x - 1];
}

static void reset_screen( union Screen *const screen )
{
    usize const cellsCount = ARR_COUNT( screen->charsRaw );
    for ( usize idx = 0; idx < cellsCount; ++idx )
    {
        screen->charsRaw[idx] = default_character();
    }
}


static bool character_equals( struct Character const lhs, struct Character const rhs )
{
    return lhs.unicode == rhs.unicode && properties_equals( lhs.properties, rhs.properties );
}


static vec2u16 get_screen_size( void const *handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( (HANDLE)handle, &info );

    u16 const newscreenH = info.srWindow.Bottom - info.srWindow.Top + 1;
    u16 const newScreenW = info.srWindow.Right - info.srWindow.Left + 1;

    return VEC2U16( newScreenW, newscreenH );
}


bool console_screen_init( void const *handle )
{
    vec2u16 const screenSize = get_screen_size( handle );

    s_screenData = (struct ScreenData ) {
        .handle = handle,
        .realScreenSize = screenSize,
        .gameScreenSize = (union screensize) {
            .h = min( screenSize.h, GAME_SIZE_HEIGHT ),
            .w = min( screenSize.w, GAME_SIZE_WIDTH )
        },
        .currPos = (union screenpos) {.y = 1, .x = 1 },
        .currProperties = default_properties(),
        // .screen
    };

    reset_screen( &s_screenData.screen );

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

    for ( usize idx = 0; idx < bufferSize; ++idx )
    {
        // If we are reaching the end of the game screen, we don't want to continue on the next line.
        if ( s_screenData.currPos.x > s_screenData.gameScreenSize.w ) break;

        struct Character *const character = get_character_at_pos( s_screenData.currPos );
        *character = (struct Character) {
            .unicode = buffer[idx],
            .properties = s_screenData.currProperties
        };
        character->properties.flags |= SpecialFlags_NEEDS_REFRESH;

        s_screenData.currPos.x += 1;
    }

    return bufferSize;
}


void console_clear( void )
{
    reset_screen( &s_screenData.screen );
}


// 


struct Properties properties_make( enum Color const color, enum Brightness const brightness, enum AttrFlags const flags )
{
    return (struct Properties) {
        .color = color | brightness,
        .flags = flags
    };
}


void console_set_properties( struct Properties const properties )
{
    s_screenData.currProperties = properties;
}


static inline bool is_bold( struct Properties const properties )
{
    return ( properties.flags & AttrFlags_BOLD ) == AttrFlags_BOLD;
}

static inline bool is_faint( struct Properties const properties )
{
    return ( properties.flags & AttrFlags_FAINT ) == AttrFlags_FAINT;
}

// [ ... ] do the rest

static u8 get_fgcolor_code( struct Properties const properties )
{
    bool const isBrightForeground = ( ( properties.color & Brightness_FG ) != 0 );
    u8 const brightParam = isBrightForeground ? BRIGHT_VALUE : 0;

    static_assert( ColorFG_MaskAll == 0b00111000, "If the Mask has changed, you need to change this conversion as well" );
    u8 const colorCode = ( properties.color & ColorFG_MaskAll ) >> 3;

    return FOREGROUND_COLOR_BASE + brightParam + colorCode;
}


static u8 get_bgcolor_code( struct Properties const properties )
{
    bool const isBrightForeground = ( ( properties.color & Brightness_BG ) != 0 );
    u8 const brightParam = isBrightForeground ? BRIGHT_VALUE : 0;

    static_assert( ColorBG_MaskAll == 0b00000111, "If the Mask has changed, you need to change this conversion as well" );
    u8 const colorCode = ( properties.color & ColorBG_MaskAll );

    return BACKGROUND_COLOR_BASE + brightParam + colorCode;    
}


static int generate_color_sequence( struct Properties const properties, utf16 *const buffer, usize const bufferSize )
{
    return swprintf( buffer, bufferSize, L"\x1b[%u;%um", get_fgcolor_code( properties ), get_bgcolor_code( properties ) );
}


static int generate_attributes_sequence( struct Properties const properties, utf16 *const buffer, usize const bufferSize )
{
    // Start with a 0 to reset the old attributes first.
    int nbWritten = swprintf( buffer, bufferSize, L"\x1b[0" );

    if ( is_bold( properties ) )  { nbWritten += swprintf( buffer + nbWritten, bufferSize - nbWritten, L";1" ); }
    if ( is_faint( properties ) ) { nbWritten += swprintf( buffer + nbWritten, bufferSize - nbWritten, L";2" ); }
    // TODO : Complete the different edge cases [...]

    nbWritten += swprintf( buffer + nbWritten, bufferSize - nbWritten, L"m" );

    return nbWritten;
}


int generate_style_sequence( struct Properties const properties, utf16 *const buffer, usize const bufferSize )
{
    int nbWritten = 0;
    nbWritten += generate_attributes_sequence( properties, buffer + nbWritten, bufferSize - nbWritten );
    nbWritten += generate_color_sequence( properties, buffer + nbWritten, bufferSize - nbWritten );
    return nbWritten;
}


void console_refresh_v2( void )
{
    static utf16 refreshBuffer[8092] = {};
    usize bufPos = 0;

    // default State
    struct Properties currProperties = default_properties();
    union screenpos cursorPos = (union screenpos) { .y = 1, .x = 1 };

    for ( usize y = 0; y < s_screenData.gameScreenSize.h; ++y )
    {
        for ( usize x = 0; x < s_screenData.gameScreenSize.w; ++x )
        {
            struct Character *character = &s_screenData.screen.charsArray[y][x];

            if ( ( character->properties.flags & SpecialFlags_NEEDS_REFRESH ) == 0 ) continue;

            union screenpos const newPos = (union screenpos) {
                .y = y + 1,
                .x = x + 1
            };

            // Ensure first that the cursor is in good position. If not, update it accordingly.
            if ( cursorPos.raw != newPos.raw )
            {
                bufPos += snwprintf( refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos, L"\x1B[%u;%uH", newPos.y, newPos.x );
                cursorPos = newPos;
            }

            // Then check if the style needs to be adjusted before writing the unicode character
            if ( !properties_equals( currProperties, character->properties ) )
            {
                bufPos += generate_style_sequence( character->properties, refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos );
                currProperties = character->properties;
            }

            // Write the new unicode character
            bufPos += snwprintf( refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos, L"%lc", character->unicode );
            cursorPos.x += 1;

            character->properties.flags &= ~SpecialFlags_NEEDS_REFRESH;
        }
    }

    if ( bufPos > 0 )
    {
        // Set the cursor back to 1;1 and the style to default in order to assume that for each refresh,
        // We are starting clean.
        snwprintf( refreshBuffer + bufPos, ARR_COUNT( refreshBuffer ) - bufPos, L"\x1B[1;1H\x1B[0;0m" );
        wprintf( refreshBuffer );
    }
}



// [...]


void console_set_pos( screenpos_deprecated const pos )
{
    s_screenData.currPos = (union screenpos) { .y = pos.y, .x = pos.x };
}

screenpos_deprecated console_pos( void )
{
    return SCREENPOS_DEPRECATED( s_screenData.currPos.x, s_screenData.currPos.y );
}


static OnScreenResizeCallback s_callbacks[RESIZE_CALLBACKS_MAX_COUNT] = {};
static u32 s_callbackCount = 0;

void console_on_screen_resize( vec2u16 const new )
{
    vec2u16 const old = s_screenData.realScreenSize;
    if ( old.x == new.x && old.y == new.y ) return;

    s_screenData.gameScreenSize = (union screensize) {
        .h = min( new.h, GAME_SIZE_HEIGHT ),
        .w = min( new.w, GAME_SIZE_WIDTH )
    };

    if ( old.x < new.x && old.x <= GAME_SIZE_WIDTH )
    {
        // draw the missing columns, erased when the window has been reduced.
    }

    if ( old.y < new.y && old.y <= GAME_SIZE_HEIGHT )
    {
        // draw the missing lines, erased when console size has been reduced.
    }

    s_screenData.realScreenSize = new;

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
// static nsecond s_resizeTimestamp = 0;

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

  //  nsecond const timestamp = time_get_timestamp_nsec();

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
	nsecond const timestamp = time_get_timestamp_nsec();
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
    return s_screenData.realScreenSize;
//    return s_currentSize;
}
