#include "terminal/terminal.h"
#include "terminal/internal/terminal_sequence.h"
#include "terminal/terminal_character.h"
#include "game.h"
#include "events.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


struct Screen
{
    union
    {
        struct Character raw[GAME_SIZE_AREA];
        struct Character content[GAME_SIZE_HEIGHT][GAME_SIZE_WIDTH];
    };
};


struct ScreenInfo
{
    struct Screen screen;

    // While the first is the current size of the terminal,
    // The second one is limited to the boundaries of the game: 120x30.
    screensize size;
    screensize supportedGameSize;
};


static struct ScreenInfo s_screenInfo;


static screensize game_size_from_screen( screensize const screenSize )
{
    return (screensize) {
        .h = min( screenSize.h, GAME_SIZE_HEIGHT ),
        .w = min( screenSize.w, GAME_SIZE_WIDTH )
    };
}


static screensize get_screen_size( void const *handle )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( (HANDLE)handle, &info );

    u16 const newscreenH = info.srWindow.Bottom - info.srWindow.Top + 1;
    u16 const newScreenW = info.srWindow.Right - info.srWindow.Left + 1;

    return (screensize) { .w = newScreenW, .h = newscreenH };
}


static struct Character *const get_character_at_pos( screenpos const pos )
{
    // While our indexes begin at 0:0, a screenpos starts at 1:1
    return &s_screenInfo.screen.content[pos.y - 1][pos.x - 1];
}


bool term_screen_init( void const *handle )
{
    screensize const screenSize = get_screen_size( handle );

    s_screenInfo.size = screenSize;
    s_screenInfo.supportedGameSize = game_size_from_screen( screenSize );
    term_clear();

    return true;
}


int term_write( utf16 const *format, ... )
{
    static utf16 buffer[TERM_WRITE_BUFFER_SIZE] = {};

    va_list args;
	va_start( args, format );
    int const bufferSize = vsnwprintf( buffer, ARR_COUNT( buffer ), format, args );
	va_end( args );

    assert( bufferSize > 0 ); // Otherwise, we may have busted the limit of the buffer, or a bad format has been given.

    u16 const maxWidth = s_screenInfo.supportedGameSize.w;

    for ( usize idx = 0; idx < bufferSize; ++idx )
    {
        screenpos const cursorPos = cursor_pos();
  
        // We have reached the end of the game screen, and we don't want to continue on the next line either.
        // So stop prematurely here.
        if ( cursorPos.x > maxWidth ) break;

        struct Character *const character = get_character_at_pos( cursorPos );

        *character = (struct Character) {
            .unicode = buffer[idx],
            .style = style_current()
        };
        character_mark_as_refresh_needed( character );
        cursor_move_right_by( 1 );
    }

    return bufferSize;
}


void term_clear( void )
{
    for ( usize idx = 0; idx < ARR_COUNT( s_screenInfo.screen.raw ); ++idx )
    {
        s_screenInfo.screen.raw[idx] = character_default();
    }
}


void term_refresh( void )
{
    static utf16 buffer[TERM_REFRESH_BUFFER_SIZE] = {};
    usize const bufTotalSize = ARR_COUNT( buffer );
    usize bufPos = 0;

    screensize const gameSize = s_screenInfo.supportedGameSize;
    struct Style style = STYLE_DEFAULT;
    screenpos cursorPos = (screenpos) { .y = 1, .x = 1 };

    for ( usize y = 0; y < gameSize.h; ++y )
    {
        for ( usize x = 0; x < gameSize.w; ++x )
        {
            struct Character *character = &s_screenInfo.screen.content[y][x];

            if ( !character_needs_refresh( *character ) )
                continue;

            // Ensure first that the cursor is in good position. If not, update it accordingly.
            screenpos const targetPos = (screenpos) { .y = y + 1, .x = x + 1 };
            if ( cursorPos.raw != targetPos.raw )
            {
                bufPos += term_sequence_set_cursor_pos( buffer + bufPos, bufTotalSize - bufPos, targetPos );
                cursorPos = targetPos;
            }

            // Then check if the style needs to be adjusted before writing the unicode character
            if ( !style_equals( style, character->style ) )
            {
                bufPos += term_sequence_set_style( buffer + bufPos, bufTotalSize - bufPos, character->style );
                style = character->style;
            }

            // Write the new unicode character
            bufPos += snwprintf( buffer + bufPos, bufTotalSize - bufPos, L"%lc", character->unicode );
            cursorPos.x += 1;

            character_refreshed( character );
        }
    }

    if ( bufPos > 0 )
    {
        bufPos += term_sequence_reset_cursor_pos( buffer + bufPos, bufTotalSize - bufPos );
        bufPos += term_sequence_reset_style( buffer + bufPos, bufTotalSize - bufPos );

        wprintf( buffer );
    }
}


void term_on_resize( screensize const newSize )
{
    screensize const old = s_screenInfo.size;
    if ( old.w == newSize.w && old.h == newSize.h ) return;

    if ( old.h > newSize.h && newSize.h < GAME_SIZE_HEIGHT )
    {
        for ( usize lineHeight = newSize.h; lineHeight < old.h; lineHeight++ )
        {
            for ( usize x = 0; x < GAME_SIZE_WIDTH; ++x )
            {
                struct Character *character = get_character_at_pos( SCREENPOS( x + 1, lineHeight + 1 ) );
                character_mark_as_refresh_needed( character );
            }
        }
    }

    if ( old.w > newSize.w && newSize.w < GAME_SIZE_WIDTH )
    {
        for ( usize lineHeight = 0; lineHeight < GAME_SIZE_HEIGHT; lineHeight++ )
        {
            for ( usize x = newSize.w; x < old.w; ++x )
            {
                struct Character *character = get_character_at_pos( SCREENPOS( x + 1, lineHeight + 1 ) );
                character_mark_as_refresh_needed( character );
            }
        }
    }

    s_screenInfo.size = newSize;
    s_screenInfo.supportedGameSize = game_size_from_screen( newSize );

    struct Event screenResized = (struct Event) {
        .type = EventType_SCREEN_RESIZED,
        .screenResized = (struct EventScreenResized) {
            .size = s_screenInfo.size
        }
    };
    event_trigger( &screenResized );
//    components_on_screen_resize( s_screenInfo.size );
}


struct Character term_character_buffered_at_pos( screenpos const pos )
{
    return s_screenInfo.screen.content[pos.y - 1][pos.x - 1];
}


screensize term_size( void )
{
    return s_screenInfo.size;
}
