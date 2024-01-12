#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "terminal/terminal_style.h"

int term_sequence_reset_style( utf16 *outBuffer, usize bufferSize );
int term_sequence_reset_cursor_pos( utf16 *outBuffer, usize bufferSize );

int term_sequence_set_properties( utf16 *outBuffer, usize bufferSize, byte properties );
int term_sequence_set_color( utf16 *outBuffer, usize bufferSize, termcolor color );
int term_sequence_set_style( utf16 *outBuffer, usize bufferSize, struct TermStyle style );
int term_sequence_set_cursor_pos( utf16 *outBuffer, usize bufferSize, screenpos pos );
