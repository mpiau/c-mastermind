#pragma once

#include "core/core.h"

enum BGColor
{
    BGColor_BLACK   = 0b00000000,
    BGColor_RED     = 0b00000001,
    BGColor_GREEN   = 0b00000010,
    BGColor_YELLOW  = 0b00000011,
    BGColor_BLUE    = 0b00000100,
    BGColor_MAGENTA = 0b00000101,
    BGColor_CYAN    = 0b00000110,
    BGColor_WHITE   = 0b00000111,

    BGColor_MaskBright = 0b00001000,

    BGColor_BRIGHT_BLACK   = BGColor_MaskBright | BGColor_BLACK,
    BGColor_BRIGHT_RED     = BGColor_MaskBright | BGColor_RED,
    BGColor_BRIGHT_GREEN   = BGColor_MaskBright | BGColor_GREEN,
    BGColor_BRIGHT_YELLOW  = BGColor_MaskBright | BGColor_YELLOW,
    BGColor_BRIGHT_BLUE    = BGColor_MaskBright | BGColor_BLUE,
    BGColor_BRIGHT_MAGENTA = BGColor_MaskBright | BGColor_MAGENTA,
    BGColor_BRIGHT_CYAN    = BGColor_MaskBright | BGColor_CYAN,
    BGColor_BRIGHT_WHITE   = BGColor_MaskBright | BGColor_WHITE,
};

enum FGColor
{
    FGColor_BLACK   = 0b00000000,
    FGColor_RED     = 0b00010000,
    FGColor_GREEN   = 0b00100000,
    FGColor_YELLOW  = 0b00110000,
    FGColor_BLUE    = 0b01000000,
    FGColor_MAGENTA = 0b01010000,
    FGColor_CYAN    = 0b01100000,
    FGColor_WHITE   = 0b01110000,

    FGColor_MaskBright = 0b10000000,

    FGColor_BRIGHT_BLACK   = FGColor_MaskBright | FGColor_BLACK,
    FGColor_BRIGHT_RED     = FGColor_MaskBright | FGColor_RED,
    FGColor_BRIGHT_GREEN   = FGColor_MaskBright | FGColor_GREEN,
    FGColor_BRIGHT_YELLOW  = FGColor_MaskBright | FGColor_YELLOW,
    FGColor_BRIGHT_BLUE    = FGColor_MaskBright | FGColor_BLUE,
    FGColor_BRIGHT_MAGENTA = FGColor_MaskBright | FGColor_MAGENTA,
    FGColor_BRIGHT_CYAN    = FGColor_MaskBright | FGColor_CYAN,
    FGColor_BRIGHT_WHITE   = FGColor_MaskBright | FGColor_WHITE,
};

enum Color
{
    Color_DEFAULT = FGColor_WHITE | BGColor_BLACK,
};

typedef byte termcolor;

byte color_foreground_termcode( termcolor color );
byte color_background_termcode( termcolor color );
