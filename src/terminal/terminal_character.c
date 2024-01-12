#include "terminal/terminal_character.h"

// Special property that is using the first bit of the properties field in TermStyle
// While I agree that it's a dependency that could involve some risks if not handled carefully,
// It allows us to have all the needed information for the screen handling inside 4 bytes only,
// and effectively reducing our memory footprint.
enum DisplayProperty
{
    DisplayProperty_REFRESH_NEEDED = 0b10000000
};


enum // Constants
{
    DEFAULT_UNICODE = L' '    
};


struct Character term_character_make( utf16 const unicode, struct TermStyle const style )
{
    return (struct Character) {
        .unicode = unicode,
        .style = style
    };
}


struct Character term_character_default( void )
{
    return term_character_make( DEFAULT_UNICODE, term_style_default() );
}


bool term_character_equals( struct Character const lhs, struct Character const rhs )
{
    return lhs.unicode == rhs.unicode && term_style_equals( lhs.style, rhs.style );
}


bool term_character_needs_refresh( struct Character const character )
{
    return ( character.style.properties & DisplayProperty_REFRESH_NEEDED ) != 0;
}


void term_character_refresh_needed( struct Character *const outCharacter )
{
    outCharacter->style.properties |= DisplayProperty_REFRESH_NEEDED;
}


void term_character_refreshed( struct Character *const outCharacter )
{
    outCharacter->style.properties &= ~DisplayProperty_REFRESH_NEEDED;
}
