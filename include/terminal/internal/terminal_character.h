#pragma once

#include "terminal/terminal_style.h"

struct Character
{
    utf16 unicode;
    struct Style style;
};
static_assert( sizeof( struct Character ) == 4 );


struct Character character_make( utf16 unicode, struct Style style );
struct Character character_default( void );

bool character_equals( struct Character lhs, struct Character rhs );

bool character_needs_refresh( struct Character const character );
void character_mark_as_refresh_needed( struct Character *const outCharacter );
void character_refreshed( struct Character *const outCharacter );
