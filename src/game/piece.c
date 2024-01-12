#include "game/piece.h"

#include "console/console.h"
#include "settings.h"
#include "characters_list.h"

static utf16 const S_PEG_NUM_UTF16[PegId_Count] =
{
    [PegId_RED]     = L'O',
    [PegId_GREEN]   = L'1',
    [PegId_YELLOW]  = L'2',
    [PegId_CYAN]    = L'3',
    [PegId_MAGENTA] = L'4',
    [PegId_BLUE]    = L'5',
    [PegId_WHITE]   = L'6',
    [PegId_BLACK]   = L'7'
};


static enum ConsoleColorFG get_color_from_id( enum PegId const id )
{
	switch ( id )
	{
		case PegId_RED:     return ConsoleColorFG_RED;
		case PegId_GREEN:   return ConsoleColorFG_GREEN;
		case PegId_YELLOW:  return ConsoleColorFG_YELLOW;
		case PegId_CYAN:    return ConsoleColorFG_CYAN;
		case PegId_MAGENTA: return ConsoleColorFG_MAGENTA;
		case PegId_BLUE:    return ConsoleColorFG_BLUE;
		case PegId_WHITE:   return ConsoleColorFG_WHITE;
		case PegId_BLACK:   return ConsoleColorFG_BRIGHT_BLACK;
		case PegId_Empty:   return ConsoleColorFG_BRIGHT_BLACK;
		default: assert( false );
	}
}

static utf16 get_character_from_id( enum PegId const id )
{
    if ( settings_is_color_blind_mode_enabled( ) )
    {
        return ( id == PegId_Empty ) ? UTF16C_SmallDottedCircle : S_PEG_NUM_UTF16[id];
    }
    return ( id == PegId_Empty ) ? UTF16C_SmallDottedCircle : UTF16C_BigFilledCircle;
}


void peg_draw( struct Peg const *peg, usize const ulX, usize const ulY )
{
    console_color_fg( peg->hidden ? ConsoleColorFG_BRIGHT_BLACK : get_color_from_id( peg->id ) );
    console_cursor_set_position( ulY, ulX );

    if ( peg->id == PegId_Empty )
    {
		console_draw( L",:'':." );
		console_cursor_set_position(ulY + 1, ulX );
		console_draw( L":    :" );
		console_cursor_set_position( ulY + 2, ulX );
		console_draw( L"`:,,:'" );
        return;
    }

	console_draw( L",d||b." );

    console_cursor_set_position( ulY + 1, ulX );
    if ( peg->hidden )
    {
    	console_draw( L"??????" );
    }
    else
    {
        utf16 const middleCharacter = peg->hidden ? UTF16C_QuestionMark : S_PEG_NUM_UTF16[peg->id];
        for ( usize x = 0; x < PegSize_WIDTH; ++x )
        {
            console_draw( L"%lc", middleCharacter );
        }
    }

    console_cursor_set_position( ulY + 2, ulX );
    console_draw( L"`Y||P'" );
}


void peg_draw_single_character( struct Peg const *peg, usize const posX, usize const posY )
{
    console_color_fg( peg->hidden ? ConsoleColorFG_BRIGHT_BLACK : get_color_from_id( peg->id ) );
    console_cursor_set_position( posY, posX );

    console_draw( L"%lc", peg->hidden ? UTF16C_QuestionMark : get_character_from_id( peg->id ) );
}





enum ConsoleColorFG pin_get_color( enum PinId const id )
{
	switch ( id )
	{
		case PinId_CORRECT:			  return ConsoleColorFG_RED;
		case PinId_PARTIALLY_CORRECT: return ConsoleColorFG_WHITE;
		case PinId_INCORRECT:         return ConsoleColorFG_BRIGHT_BLACK;
		default: assert( false );
	}
}
