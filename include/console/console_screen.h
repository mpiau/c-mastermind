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
enum Brightness
{
    Brightness_NONE = 0b00000000,
    Brightness_BG   = 0b01000000,
    Brightness_FG   = 0b10000000,
    Brightness_ALL  = Brightness_FG | Brightness_BG,
};

enum AttrFlags
{
    AttrFlags_NONE      = 0b00000000,
    AttrFlags_BOLD      = 0b00000001,
    AttrFlags_FAINT     = 0b00000010,
    AttrFlags_ITALIC    = 0b00000100,
    AttrFlags_UNDERLINE = 0b00001000,
    AttrFlags_BLINK     = 0b00010000,
    AttrFlags_MaskAll   = 0b00011111

    // 2 available attributes remaining
};

enum SpecialFlags
{
    SpecialFlags_NEEDS_REFRESH = 0b10000000
};

struct Properties
{
    u8 color;
    u8 flags;
};
static_assert( sizeof( struct Properties ) == 2 );


bool console_screen_init( void const *handle );
void console_screen_uninit( void );


// Write / clear in an internal buffer, that won't be displayed to the user until the refresh function is called.
int console_write( utf16 const *format, ... );
void console_clear( void );

// Draw the frame into the console (V2 to avoid a clash with the console.h for now)
void console_refresh_v2( void );

void console_on_screen_resize( vec2u16 newSize );


struct Properties properties_make( enum Color color, enum Brightness brightness, enum AttrFlags flags );
void console_set_properties( struct Properties properties );
void console_reset_style( void );

screenpos_deprecated console_pos( void );
void console_set_pos( screenpos_deprecated pos );




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
