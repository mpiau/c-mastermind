#pragma once

#include "terminal/terminal_style.h"

struct Character
{
    utf16 unicode;
    struct TermStyle style;
};
static_assert( sizeof( struct Character ) == 4 );


struct Character term_character_make( utf16 unicode, struct TermStyle style );
struct Character term_character_default( void );

bool term_character_equals( struct Character lhs, struct Character rhs );

bool term_character_needs_refresh( struct Character const character );
void term_character_refresh_needed( struct Character *const outCharacter );
void term_character_refreshed( struct Character *const outCharacter );
