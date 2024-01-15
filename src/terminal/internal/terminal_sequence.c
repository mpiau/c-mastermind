#include "terminal/internal/terminal_sequence.h"

#include "terminal/terminal.h"

#include <stdio.h>

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

    if ( attr_is_bold( properties ) )  { nbWritten += swprintf( outBuffer + nbWritten, bufferSize - nbWritten, L";1" ); }
    if ( attr_is_faint( properties ) ) { nbWritten += swprintf( outBuffer + nbWritten, bufferSize - nbWritten, L";2" ); }
    if ( attr_is_italic( properties ) ) { nbWritten += swprintf( outBuffer + nbWritten, bufferSize - nbWritten, L";3" ); }
    // TODO : Complete the different edge cases [...]
    // https://askubuntu.com/questions/528928/how-to-do-underline-bold-italic-strikethrough-color-background-and-size-i

    nbWritten += swprintf( outBuffer + nbWritten, bufferSize - nbWritten, L"m" );

    return nbWritten;
}


int term_sequence_set_color( utf16 *const outBuffer, usize const bufferSize, termcolor const color )
{
    u8 const fgColorCode = color_foreground_termcode( color );
    u8 const bgColorCode = color_background_termcode( color );

    return snwprintf( outBuffer, bufferSize, L"\x1B[%u;%um", fgColorCode, bgColorCode );
}


int term_sequence_set_style( utf16 *const outBuffer, usize const bufferSize, struct Style const style )
{
    int nbWritten = term_sequence_set_properties( outBuffer, bufferSize, style.attr );
    nbWritten += term_sequence_set_color( outBuffer + nbWritten, bufferSize - nbWritten, style.color );

    return nbWritten;
}


int term_sequence_set_cursor_pos( utf16 *const outBuffer, usize const bufferSize, screenpos const pos )
{
    return snwprintf( outBuffer, bufferSize, L"\x1B[%u;%uH", pos.y, pos.x );
}
