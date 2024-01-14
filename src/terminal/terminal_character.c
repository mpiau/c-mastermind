#include "terminal/terminal_character.h"

// Special attribute that is using the first bit of the attributes field of struct Style
// While I agree that it's a dependency that could involve some risks if not handled carefully,
// It allows us to have all the needed information for the screen handling inside 4 bytes only,
// and effectively reducing our memory footprint.
enum AttrInternal
{
    AttrInternal_REFRESH_NEEDED = 0b10000000
};


enum // Constants
{
    DEFAULT_UNICODE = L' '    
};


struct Character character_make( utf16 const unicode, struct Style const style )
{
    assert( ( style.attr & AttrInternal_REFRESH_NEEDED ) == 0 );

    return (struct Character) {
        .unicode = unicode,
        .style = style
    };
}


struct Character character_default( void )
{
    return character_make( DEFAULT_UNICODE, STYLE_DEFAULT );
}


bool character_equals( struct Character const lhs, struct Character const rhs )
{
    return lhs.unicode == rhs.unicode && style_equals( lhs.style, rhs.style );
}


bool character_needs_refresh( struct Character const character )
{
    return ( character.style.attr & AttrInternal_REFRESH_NEEDED ) != 0;
}


void character_mark_as_refresh_needed( struct Character *const outCharacter )
{
    outCharacter->style.attr |= AttrInternal_REFRESH_NEEDED;
}


void character_refreshed( struct Character *const outCharacter )
{
    outCharacter->style.attr &= ~AttrInternal_REFRESH_NEEDED;
}
