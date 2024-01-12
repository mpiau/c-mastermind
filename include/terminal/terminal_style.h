#pragma once

#include "core_types.h"

enum BackgroundColor
{
    BackgroundColor_BLACK   = 0b00000000,
    BackgroundColor_RED     = 0b00000001,
    BackgroundColor_GREEN   = 0b00000010,
    BackgroundColor_YELLOW  = 0b00000011,
    BackgroundColor_BLUE    = 0b00000100,
    BackgroundColor_MAGENTA = 0b00000101,
    BackgroundColor_CYAN    = 0b00000110,
    BackgroundColor_WHITE   = 0b00000111,

    // Useful when we need to separate the background color from the rest of the byte
    BackgroundColor_BitmaskAll = 0b00000111,
};

enum ForegroundColor
{
    ForegroundColor_BLACK   = 0b00000000,
    ForegroundColor_RED     = 0b00001000,
    ForegroundColor_GREEN   = 0b00010000,
    ForegroundColor_YELLOW  = 0b00011000,
    ForegroundColor_BLUE    = 0b00100000,
    ForegroundColor_MAGENTA = 0b00101000,
    ForegroundColor_CYAN    = 0b00110000,
    ForegroundColor_WHITE   = 0b00111000,

    // Useful when we need to separate the foreground color from the rest of the byte
    ForegroundColor_BitmaskAll = 0b00111000,
};

enum ColorBrightness
{
    ColorBrightness_NONE       = 0b00000000,
    ColorBrightness_BACKGROUND = 0b01000000,
    ColorBrightness_FOREGROUND = 0b10000000,
    ColorBrightness_ALL        = ColorBrightness_FOREGROUND | ColorBrightness_BACKGROUND,
};

enum // Constants
{
    COLOR_FILLED_BLACK   = ForegroundColor_BLACK   | BackgroundColor_BLACK,
    COLOR_FILLED_RED     = ForegroundColor_RED     | BackgroundColor_RED,
    COLOR_FILLED_GREEN   = ForegroundColor_GREEN   | BackgroundColor_GREEN,
    COLOR_FILLED_YELLOW  = ForegroundColor_YELLOW  | BackgroundColor_YELLOW,
    COLOR_FILLED_BLUE    = ForegroundColor_BLUE    | BackgroundColor_BLUE,
    COLOR_FILLED_MAGENTA = ForegroundColor_MAGENTA | BackgroundColor_MAGENTA,
    COLOR_FILLED_CYAN    = ForegroundColor_CYAN    | BackgroundColor_CYAN,
    COLOR_FILLED_WHITE   = ForegroundColor_WHITE   | BackgroundColor_WHITE,

    // Default terminal color
    COLOR_DEFAULT      = ForegroundColor_WHITE | BackgroundColor_BLACK,

    // Color that is used often enough in the game to have its own constant.
    COLOR_BRIGHT_BLACK = ForegroundColor_BLACK | ColorBrightness_FOREGROUND,
};

enum Properties
{
    Properties_NONE      = 0b00000000,
    Properties_BOLD      = 0b00000001,
    Properties_FAINT     = 0b00000010,
    Properties_ITALIC    = 0b00000100,
    Properties_UNDERLINE = 0b00001000,
    Properties_BLINK     = 0b00010000,
    Properties_ALL       = 0b00011111,

    // Available : 0b00100000
    // Available : 0b01000000

    // Important
    // 0b10000000 is a reserved value that shouldn't be set in this enum
};


typedef byte termcolor;

struct TermStyle
{
    termcolor color;
    byte      properties;
};

struct TermStyle term_style_make( termcolor color, enum Properties properties );

struct TermStyle term_style_default( void );
struct TermStyle term_style_current( void );
void term_style_set_current( struct TermStyle style );
bool term_style_equals( struct TermStyle lhs, struct TermStyle rhs );

bool term_style_is_bold( byte properties );
bool term_style_is_faint( byte properties );
bool term_style_is_italic( byte properties );
bool term_style_is_underline( byte properties );
