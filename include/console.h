#pragma once

#include "core_types.h"
#include "core_unions.h"

#include <windows.h> // Temp only, will remove later but convenient for the moment

#include "console/console_screen.h"

enum ConsoleColorFG
{
    ConsoleColorFG_BLACK   = 30,
    ConsoleColorFG_RED     = 31,
    ConsoleColorFG_GREEN   = 32,
    ConsoleColorFG_YELLOW  = 33,
    ConsoleColorFG_BLUE    = 34,
    ConsoleColorFG_MAGENTA = 35,
    ConsoleColorFG_CYAN    = 36,
    ConsoleColorFG_WHITE   = 37,
    
    ConsoleColorFG_BRIGHT_BLACK   = 90,
    ConsoleColorFG_BRIGHT_RED     = 91,
    ConsoleColorFG_BRIGHT_GREEN   = 92,
    ConsoleColorFG_BRIGHT_YELLOW  = 93,
    ConsoleColorFG_BRIGHT_BLUE    = 94,
    ConsoleColorFG_BRIGHT_MAGENTA = 95,
    ConsoleColorFG_BRIGHT_CYAN    = 96,
    ConsoleColorFG_BRIGHT_WHITE   = 97,

    ConsoleColorFG_Default = 39
};

enum ConsoleColorBG
{
    ConsoleColorBG_BLACK   = 40,
    ConsoleColorBG_RED     = 41,
    ConsoleColorBG_GREEN   = 42,
    ConsoleColorBG_YELLOW  = 43,
    ConsoleColorBG_BLUE    = 44,
    ConsoleColorBG_MAGENTA = 45,
    ConsoleColorBG_CYAN    = 46,
    ConsoleColorBG_WHITE   = 47,
    
    ConsoleColorBG_BRIGHT_BLACK   = 100,
    ConsoleColorBG_BRIGHT_RED     = 101,
    ConsoleColorBG_BRIGHT_GREEN   = 102,
    ConsoleColorBG_BRIGHT_YELLOW  = 103,
    ConsoleColorBG_BRIGHT_BLUE    = 104,
    ConsoleColorBG_BRIGHT_MAGENTA = 105,
    ConsoleColorBG_BRIGHT_CYAN    = 106,
    ConsoleColorBG_BRIGHT_WHITE   = 107,

    ConsoleColorBG_Default = 49
};


bool console_global_init( char const *optTitle, bool onDedicatedConsole );
void console_global_uninit( void );
bool console_is_global_init( void );


HANDLE console_input_handle( void );
HANDLE console_output_handle( void );


bool console_set_title( char const *title );

// Cursor
void console_cursor_hide( void );
void console_cursor_show( void );
void console_cursor_start_blinking( void );
void console_cursor_stop_blinking( void );
void console_cursor_set_position( short y, short x );
void console_cursor_setpos( screenpos_deprecated position );
void console_cursor_move_up_by( short n );
void console_cursor_move_down_by( short n );
void console_cursor_move_left_by( short n );
void console_cursor_move_right_by( short n );

// Drawing Mode
void console_line_drawing_mode_enter( void );
void console_line_drawing_mode_exit( void );

void console_alternate_buffer_enter( void );
void console_alternate_buffer_exit( void );

// Colors
void console_color_reset( void );

void console_color_bold( void );
void console_color_no_bold( void );
void console_color_underline( void );
void console_color_no_underline( void );

void console_color_negative( void ); // Swap background and foreground
void console_color_positive( void ); // Swap back to normal

//void console_color( enum ConsoleColorFG fgColor, enum ConsoleColorBG bgColor );
void console_color_fg( enum ConsoleColorFG fgColor );
void console_color_bg( enum ConsoleColorBG bgColor );


//vec2u16 console_screen_get_size( HANDLE handle );
void console_screen_clear( void );

void console_refresh( void );
int console_draw( utf16 const *format, ... );

void console_setpos( screenpos_deprecated pos );
