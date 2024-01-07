#pragma once

#include "core_types.h"
#include "core_unions.h"
#include "game.h"

enum AttrColor
{
    AttrColor_DEFAULT    = 0x0000000000000000,

    // background part
    AttrColor_BLACK_BG   = 0x0000000000000001,
    AttrColor_RED_BG     = 0x0000000000000010,
    AttrColor_GREEN_BG   = 0x0000000000000100,
    AttrColor_YELLOW_BG  = 0x0000000000001000,
    AttrColor_BLUE_BG    = 0x0000000000010000,
    AttrColor_MAGENTA_BG = 0x0000000000100000,
    AttrColor_CYAN_BG    = 0x0000000001000000,
    AttrColor_WHITE_BG   = 0x0000000010000000,
    AttrColor_MaskBackground = 0x0000000011111111,

    // Foreground part
    AttrColor_BLACK_FG   = 0x0000000100000000,
    AttrColor_RED_FG     = 0x0000001000000000,
    AttrColor_GREEN_FG   = 0x0000010000000000,
    AttrColor_YELLOW_FG  = 0x0000100000000000,
    AttrColor_BLUE_FG    = 0x0001000000000000,
    AttrColor_MAGENTA_FG = 0x0010000000000000,
    AttrColor_CYAN_FG    = 0x0100000000000000,
    AttrColor_WHITE_FG   = 0x1000000000000000,
    AttrColor_MaskForeground = 0x1111111100000000,

    // Both foreground and background
    AttrColor_BLACK_FULL   = AttrColor_BLACK_FG   | AttrColor_BLACK_BG,
    AttrColor_RED_FULL     = AttrColor_RED_FG     | AttrColor_RED_BG,
    AttrColor_GREEN_FULL   = AttrColor_GREEN_FG   | AttrColor_GREEN_BG,
    AttrColor_YELLOW_FULL  = AttrColor_YELLOW_FG  | AttrColor_YELLOW_BG,
    AttrColor_BLUE_FULL    = AttrColor_BLUE_FG    | AttrColor_BLUE_BG,
    AttrColor_MAGENTA_FULL = AttrColor_MAGENTA_FG | AttrColor_MAGENTA_BG,
    AttrColor_CYAN_FULL    = AttrColor_CYAN_FG    | AttrColor_CYAN_BG,
    AttrColor_WHITE_FULL   = AttrColor_WHITE_FG   | AttrColor_WHITE_BG,
};

enum AttrStyle
{
    AttrStyle_DEFAULT       = 0x00000000,
    AttrStyle_BOLD          = 0x00000001,
    AttrStyle_UNDERLINE     = 0x00000010,
    AttrStyle_STRIKETHROUGH = 0x00000100,
    AttrStyle_ITALIC        = 0x00001000,
    AttrStyle_BLINK         = 0x00010000,
    AttrStyle_REVERSE       = 0x00100000,
    AttrStyle_INVISIBLE     = 0x01000000,
};

enum AttrShade
{
    AttrShade_DEFAULT = 0,
    AttrShade_DARK,
    AttrShade_BRIGHT
};


struct Attr
{
    enum AttrColor color;
    enum AttrStyle style;
    enum AttrShade shade;
};

#define ATTR( _color, _style, _shade ) ( struct Attr ) { .color = _color, .style = _style, .shade = _shade }


bool console_screen_init( void const *handle );
void console_screen_uninit( void );


// Write / clear in an internal buffer, that won't be displayed to the user until the refresh function is called.
int console_write( utf16 const *format, ... );
void console_clear( void );

// Draw the frame into the console (V2 to avoid a clash with the console.h for now)
void console_refresh_v2( void );

void console_on_screen_resize( vec2u16 newSize );


// Set the active attributes that will be applied on the next written character.
void console_set_color( enum AttrColor color );
void console_set_style( enum AttrStyle style );
void console_set_shade( enum AttrShade shade );
void console_set_attr( struct Attr attributes );

void console_reset_attr( void );

// Retrieve the active attributes that will be applied on the next written character.
enum AttrColor console_color( void );
enum AttrStyle console_style( void );
enum AttrShade console_shade( void );
struct Attr console_attr( void );

screenpos console_cpos( void );
void console_set_cpos( screenpos pos );




// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////



typedef void ( *OnScreenResizeCallback )( vec2u16 oldSize, vec2u16 newSize );

// bool console_screen_init( void const *handle ); /* HANDLE */
void console_screen_uninit( void );



void console_screen_frame( void );
bool console_screen_resize( vec2u16 newSize );
//bool console_screen_is_being_resized( void );

bool console_screen_is_too_small( void );
bool console_screen_is_width_too_small( void );
bool console_screen_is_height_too_small( void );

bool console_screen_register_on_resize_callback( OnScreenResizeCallback callback );
vec2u16 console_screen_get_size( void );
