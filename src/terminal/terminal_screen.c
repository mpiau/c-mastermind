#include "terminal/terminal_screen.h"
#include "terminal/terminal_sequence.h"
#include "terminal/terminal_character.h"
#include "game.h"
#include "widgets/widget.h"

#include <stdio.h>
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

    screenpos cursorPos;
/*
    void const *handle;
*/
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

    s_screenInfo = (struct ScreenInfo) {
        .size = screenSize,
        .supportedGameSize = game_size_from_screen( screenSize ),
        .cursorPos = (screenpos) {.y = 1, .x = 1 },
        // .screen - initialized below
    };

    term_screen_clear();

    return true;
}


int term_screen_write( utf16 const *format, ... )
{
    static utf16 buffer[TERM_SCREEN_WRITE_MAXIMUM_BUFFER] = {};

    va_list args;
	va_start( args, format );
    int const bufferSize = vsnwprintf( buffer, ARR_COUNT( buffer ), format, args );
	va_end( args );

    assert( bufferSize > 0 ); // Otherwise, we may have busted the limit of the buffer, or a bad format has been given.

    u16 const maxWidth = s_screenInfo.supportedGameSize.w;
    screenpos *const cursorPos = &s_screenInfo.cursorPos;

    for ( usize idx = 0; idx < bufferSize; ++idx )
    {
        // We have reached the end of the game screen, and we don't want to continue on the next line either.
        // So stop prematurely here.
        // @mpiau note: Perhaps this is a feedback we could give to the user ?
        if ( cursorPos->x > maxWidth ) break;

        struct Character *const character = get_character_at_pos( *cursorPos );

        *character = (struct Character) {
            .unicode = buffer[idx],
            .style = term_style_current()
        };
        term_character_refresh_needed( character );

        cursorPos->x += 1;
    }

    return bufferSize;
}


void term_screen_clear( void )
{
    for ( usize idx = 0; idx < ARR_COUNT( s_screenInfo.screen.raw ); ++idx )
    {
        s_screenInfo.screen.raw[idx] = term_character_default();
    }
}


void term_screen_refresh( void )
{
    static utf16 buffer[TERM_SCREEN_REFRESH_MAXIMUM_BUFFER] = {};
    usize const bufTotalSize = ARR_COUNT( buffer );
    usize bufPos = 0;

    screensize const gameSize = s_screenInfo.supportedGameSize;
    struct TermStyle style = term_style_default();
    screenpos cursorPos = (screenpos) { .y = 1, .x = 1 };

    for ( usize y = 0; y < gameSize.h; ++y )
    {
        for ( usize x = 0; x < gameSize.w; ++x )
        {
            struct Character *character = &s_screenInfo.screen.content[y][x];

            if ( !term_character_needs_refresh( *character ) )
                continue;

            // Ensure first that the cursor is in good position. If not, update it accordingly.
            screenpos const targetPos = (screenpos) { .y = y + 1, .x = x + 1 };
            if ( cursorPos.raw != targetPos.raw )
            {
                bufPos += term_sequence_set_cursor_pos( buffer + bufPos, bufTotalSize - bufPos, targetPos );
                cursorPos = targetPos;
            }

            // Then check if the style needs to be adjusted before writing the unicode character
            if ( !term_style_equals( style, character->style ) )
            {
                bufPos += term_sequence_set_style( buffer + bufPos, bufTotalSize - bufPos, character->style );
                style = character->style;
            }

            // Write the new unicode character
            bufPos += snwprintf( buffer + bufPos, bufTotalSize - bufPos, L"%lc", character->unicode );
            cursorPos.x += 1;

            term_character_refreshed( character );
        }
    }

    if ( bufPos > 0 )
    {
        bufPos += term_sequence_reset_cursor_pos( buffer + bufPos, bufTotalSize - bufPos );
        bufPos += term_sequence_reset_style( buffer + bufPos, bufTotalSize - bufPos );

        wprintf( buffer );
    }
}


void term_screen_on_resize( screensize const newSize )
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
                term_character_refresh_needed( character );
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
                term_character_refresh_needed( character );
            }
        }
    }

    s_screenInfo.size = newSize;
    s_screenInfo.supportedGameSize = game_size_from_screen( newSize );
    components_on_screen_resize( s_screenInfo.size );
}


screensize term_screen_current_size( void )
{
    return s_screenInfo.size;
}


screenpos term_screen_cursor_pos( void )
{
    return s_screenInfo.cursorPos;
}


void term_screen_set_cursor_pos( screenpos const pos )
{
    s_screenInfo.cursorPos = pos;
}
