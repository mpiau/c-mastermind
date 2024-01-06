#pragma once

#include "core_types.h"
#include "core_unions.h"
#include "game.h"

enum AttrColor
{
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

    // Some useful masks
    AttrColor_DEFAULT = AttrColor_WHITE_FG | AttrColor_BLACK_BG,
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

struct Screen;

struct Screen *console_screen_create( void *handle );
void console_screen_destroy( struct Screen *screen );


// Write / clear in an internal buffer, that won't be displayed to the user until the refresh function is called.
void console_write( struct Screen *screen, utf16 const *format, ... );
void console_clear( struct Screen *screen );

// Draw the frame into the console (V2 to avoid a clash with the console.h for now)
void console_refresh_v2( struct Screen *screen );


// Set the active attributes that will be applied on the next written character.
void console_set_color( struct Screen *screen, enum AttrColor color );
void console_set_style( struct Screen *screen, enum AttrStyle style );
void console_set_shade( struct Screen *screen, enum AttrShade shade );
bool console_set_attributes( struct Screen *screen, enum AttrColor const *optColor, enum AttrStyle const *optStyle, enum AttrShade const *optShade );

// Retrieve the active attributes that will be applied on the next written character.
enum AttrColor console_color( struct Screen const *screen );
enum AttrStyle console_style( struct Screen const *screen );
enum AttrShade console_shade( struct Screen const *screen );
bool console_attributes( struct Screen const *screen, enum AttrColor *outOptColor, enum AttrStyle *outOptStyle, enum AttrShade *outOptShade );

screenpos console_pos( struct Screen const *screen );
void console_set_pos( struct Screen *screen, screenpos pos );




// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////
// ////////////////////////////////////////



typedef void ( *OnScreenResizeCallback )( vec2u16 oldSize, vec2u16 newSize );

bool console_screen_init( void const *handle ); /* HANDLE */
void console_screen_uninit( void );



void console_screen_frame( void );
bool console_screen_resize( vec2u16 newSize );
//bool console_screen_is_being_resized( void );

bool console_screen_is_too_small( void );
bool console_screen_is_width_too_small( void );
bool console_screen_is_height_too_small( void );

bool console_screen_register_on_resize_callback( OnScreenResizeCallback callback );
vec2u16 console_screen_get_size( void );
