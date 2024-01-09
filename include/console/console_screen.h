#pragma once

#include "core_types.h"
#include "core_unions.h"
#include "game.h"

// The foreground color displayed in the terminal
enum Color
{
    ColorBG_BLACK   = 0b00000000,
    ColorBG_RED     = 0b00000001,
    ColorBG_GREEN   = 0b00000010,
    ColorBG_YELLOW  = 0b00000011,
    ColorBG_BLUE    = 0b00000100,
    ColorBG_MAGENTA = 0b00000101,
    ColorBG_CYAN    = 0b00000110,
    ColorBG_WHITE   = 0b00000111,

    ColorFG_BLACK   = 0b00000000,
    ColorFG_RED     = 0b00001000,
    ColorFG_GREEN   = 0b00010000,
    ColorFG_YELLOW  = 0b00011000,
    ColorFG_BLUE    = 0b00100000,
    ColorFG_MAGENTA = 0b00101000,
    ColorFG_CYAN    = 0b00110000,
    ColorFG_WHITE   = 0b00111000,

    Color_DEFAULT   = ColorFG_WHITE | ColorBG_BLACK,

    ColorBG_MaskAll = 0b00000111,
    ColorFG_MaskAll = 0b00111000,
    Color_MaskAll   = ColorFG_MaskAll | ColorBG_MaskAll
};

// Control the brightness of the foreground and background color
enum ColorBrightness
{
    ColorBrightness_NONE = 0b00000000,
    ColorBrightness_BG   = 0b01000000,
    ColorBrightness_FG   = 0b10000000,
    ColorBrightness_ALL  = ColorBrightness_FG | ColorBrightness_BG,
};

// SGR (Select Graphic Rendition) parameters, setting display attributes
// You can set multiple attributes at the same time by combining them
enum DispAttr
{
    DispAttr_NONE          = 0b00000000,
    DispAttr_BOLD          = 0b00000001,
    DispAttr_FAINT         = 0b00000010,
    DispAttr_ITALIC        = 0b00000100,
    DispAttr_UNDERLINE     = 0b00001000,
    DispAttr_BLINK         = 0b00010000,
    DispAttr_STRIKETHROUGH = 0b00100000,
    DispAttr_REVERSE       = 0b01000000,
    DispAttr_INVISIBLE     = 0b10000000,

    DispAttr_MaskAll       = 0b11111111
};
static_assert( DispAttr_MaskAll <= (u8)-1 );

struct Style
{
    byte color;
    byte dispAttr;
};
static_assert( sizeof( struct Style ) == 2 );


bool console_screen_init( void const *handle );
void console_screen_uninit( void );


// Write / clear in an internal buffer, that won't be displayed to the user until the refresh function is called.
int console_write( utf16 const *format, ... );
void console_clear( void );

// Draw the frame into the console (V2 to avoid a clash with the console.h for now)
void console_refresh_v2( void );

void console_on_screen_resize( vec2u16 newSize );


struct Style style_make( enum Color color, enum ColorBrightness brightness, enum DispAttr attributes );
void console_set_style( struct Style style );
void console_reset_style( void );

screenpos console_pos( void );
void console_set_pos( screenpos pos );




// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////



typedef void ( *OnScreenResizeCallback )( vec2u16 oldSize, vec2u16 newSize );


void console_screen_frame( void );
bool console_screen_resize( vec2u16 newSize );
//bool console_screen_is_being_resized( void );

bool console_screen_is_too_small( void );
bool console_screen_is_width_too_small( void );
bool console_screen_is_height_too_small( void );

bool console_screen_register_on_resize_callback( OnScreenResizeCallback callback );
vec2u16 console_screen_get_size( void );
