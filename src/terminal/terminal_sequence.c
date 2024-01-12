#include "terminal/terminal_sequence.h"

#include <stdio.h>


enum // Constants
{
    BRIGHTNESS_CODE = 60,
    FOREGROUND_COLOR_BASE_CODE = 30,
    BACKGROUND_COLOR_BASE_CODE = 40,
};


static u8 foreground_color_sequence_code( termcolor const color )
{
    bool const isBrightForeground = ( ( color & ColorBrightness_FOREGROUND ) != 0 );
    u8 const brightnessCode = isBrightForeground ? BRIGHTNESS_CODE : 0;

    static_assert( ForegroundColor_BitmaskAll == 0b00111000, "If the Mask has changed, you need to change this conversion as well" );
    u8 const colorCode = ( color & ForegroundColor_BitmaskAll ) >> 3;

    return FOREGROUND_COLOR_BASE_CODE + brightnessCode + colorCode;
}


static u8 background_color_sequence_code( termcolor const color )
{
    bool const isBrightBackground = ( ( color & ColorBrightness_BACKGROUND ) != 0 );
    u8 const brightnessCode = isBrightBackground ? BRIGHTNESS_CODE : 0;

    static_assert( BackgroundColor_BitmaskAll == 0b00000111, "If the Mask has changed, you need to change this conversion as well" );
    u8 const colorCode = ( color & BackgroundColor_BitmaskAll );

    return BACKGROUND_COLOR_BASE_CODE + brightnessCode + colorCode;    
}


int term_sequence_reset_style( utf16 *const outBuffer, usize const bufferSize )
{
    return snwprintf( outBuffer, bufferSize, L"\x1B[0;0m" );
}


int term_sequence_reset_cursor_pos( utf16 *const outBuffer, usize const bufferSize )
{
    return term_sequence_set_cursor_pos( outBuffer, bufferSize, (screenpos) { .x = 1, .y = 1 } );
}


int term_sequence_set_properties( utf16 *const outBuffer, usize const bufferSize, byte const properties )
{
    // Start with a 0 to reset the old attributes first.
    int nbWritten = swprintf( outBuffer, bufferSize, L"\x1b[0" );

    if ( term_style_is_bold( properties ) )  { nbWritten += swprintf( outBuffer + nbWritten, bufferSize - nbWritten, L";1" ); }
    if ( term_style_is_faint( properties ) ) { nbWritten += swprintf( outBuffer + nbWritten, bufferSize - nbWritten, L";2" ); }
    // TODO : Complete the different edge cases [...]
    // https://askubuntu.com/questions/528928/how-to-do-underline-bold-italic-strikethrough-color-background-and-size-i

    nbWritten += swprintf( outBuffer + nbWritten, bufferSize - nbWritten, L"m" );

    return nbWritten;
}


int term_sequence_set_color( utf16 *const outBuffer, usize const bufferSize, termcolor const color )
{
    u8 const fgColorCode = foreground_color_sequence_code( color );
    u8 const bgColorCode = background_color_sequence_code( color );

    return snwprintf( outBuffer, bufferSize, L"\x1B[%u;%um", fgColorCode, bgColorCode );
}


int term_sequence_set_style( utf16 *const outBuffer, usize const bufferSize, struct TermStyle const style )
{
    int nbWritten = term_sequence_set_properties( outBuffer, bufferSize, style.properties );
    nbWritten += term_sequence_set_color( outBuffer + nbWritten, bufferSize - nbWritten, style.color );

    return nbWritten;
}


int term_sequence_set_cursor_pos( utf16 *const outBuffer, usize const bufferSize, screenpos const pos )
{
    return snwprintf( outBuffer, bufferSize, L"\x1B[%u;%uH", pos.y, pos.x );
}
