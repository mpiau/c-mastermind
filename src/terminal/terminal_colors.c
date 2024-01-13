#include "terminal/terminal_colors.h"

enum // Constants
{
    FGCOLOR_BASE_CODE = 30,
    BGCOLOR_BASE_CODE = 40,
    BRIGHT_CODE = 60,
};


enum Bitmask
{
    Bitmask_BG_COLOR      = 0b00000111,
    Bitmask_BG_BRIGHTNESS = 0b00001000,
    Bitmask_BG            = Bitmask_BG_COLOR | Bitmask_BG_BRIGHTNESS,

    Bitmask_FG_COLOR      = 0b01110000,
    Bitmask_FG_BRIGHTNESS = 0b10000000,
    Bitmask_FG            = Bitmask_FG_COLOR | Bitmask_FG_BRIGHTNESS,
};


byte color_foreground_termcode( termcolor const color )
{
    byte fgColor = FGCOLOR_BASE_CODE;
    fgColor += ( ( color & Bitmask_FG_COLOR ) >> 4 );

    if ( ( color & Bitmask_FG_BRIGHTNESS ) != 0 )
    {
        fgColor += BRIGHT_CODE;
    }

    return fgColor;
}


byte color_background_termcode( termcolor const color )
{
    byte bgColor = BGCOLOR_BASE_CODE;
    bgColor += ( color & Bitmask_BG_COLOR );

    if ( ( color & Bitmask_BG_BRIGHTNESS ) != 0 )
    {
        bgColor += BRIGHT_CODE;
    }

    return bgColor;
}
