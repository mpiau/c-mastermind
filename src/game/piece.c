#include "game/piece.h"

#include "terminal/terminal.h"
#include "settings.h"
#include "characters_list.h"
/*
static utf16 const S_PEG_NUM_UTF16[PegId_Count] =
{
    [PegId_BLACK]   = L'O',
    [PegId_RED]     = L'1',
    [PegId_GREEN]   = L'2',
    [PegId_YELLOW]  = L'3',
    [PegId_CYAN]    = L'4',
    [PegId_MAGENTA] = L'5',
    [PegId_BLUE]    = L'6',
    [PegId_WHITE]   = L'7',
};


static termcolor peg_color( enum PegId const id )
{
	switch ( id )
	{
		case PegId_BLACK:   return FGColor_BRIGHT_BLACK;
		case PegId_RED:     return FGColor_RED;
		case PegId_GREEN:   return FGColor_GREEN;
		case PegId_YELLOW:  return FGColor_YELLOW;
		case PegId_CYAN:    return FGColor_CYAN;
		case PegId_MAGENTA: return FGColor_MAGENTA;
		case PegId_BLUE:    return FGColor_BLUE;
		case PegId_WHITE:   return FGColor_WHITE;
		case PegId_Empty:   return FGColor_BRIGHT_BLACK;
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
    style_update( STYLE( peg->hidden ? FGColor_BRIGHT_BLACK : peg_color( peg->id ) ) );
    cursor_update_yx( ulY, ulX );

    if ( peg->id == PegId_Empty )
    {
		term_write( L",:'':." );
		cursor_update_yx(ulY + 1, ulX );
		term_write( L":    :" );
		cursor_update_yx( ulY + 2, ulX );
		term_write( L"`:,,:'" );
        return;
    }

	term_write( L",d||b." );

    cursor_update_yx( ulY + 1, ulX );
    if ( peg->hidden )
    {
    	term_write( L"??????" );
    }
    else
    {
        utf16 const middleCharacter = peg->hidden ? UTF16C_QuestionMark : S_PEG_NUM_UTF16[peg->id];
        for ( usize x = 0; x < PegSize_WIDTH; ++x )
        {
            term_write( L"%lc", middleCharacter );
        }
    }

    cursor_update_yx( ulY + 2, ulX );
    term_write( L"`Y||P'" );
}


void peg_draw_single_character( struct Peg const *peg, usize const posX, usize const posY, bool isFutureTurn )
{
    if ( isFutureTurn )
    {
        style_update( STYLE_WITH_ATTR( peg->hidden ? FGColor_BRIGHT_BLACK : peg_color( peg->id ), Attr_FAINT ) );
    }
    else
    {
        style_update( STYLE( peg->hidden ? FGColor_BRIGHT_BLACK : peg_color( peg->id ) ) );
    }
    cursor_update_yx( posY, posX );

    term_write( L"%lc", peg->hidden ? UTF16C_QuestionMark : get_character_from_id( peg->id ) );
}




struct Style pin_get_style( enum PinId const id )
{
	switch ( id )
	{
		case PinId_CORRECT:			  return STYLE( FGColor_RED );
		case PinId_PARTIALLY_CORRECT: return STYLE( FGColor_WHITE );
		case PinId_INCORRECT:         return STYLE( FGColor_BRIGHT_BLACK );
		default: assert( false );
	}
}*/


static inline bool is_empty( gamepiece const piece )
{
    return ( piece & PieceFlag_EMPTY );
}

static inline bool is_peg( gamepiece const piece )
{
    return ( piece & Piece_MaskType ) == Piece_TypePeg;
}

static inline bool is_pin( gamepiece const piece )
{
    return ( piece & Piece_MaskType ) == Piece_TypePin;
}


static termcolor get_piece_color( gamepiece const piece )
{
    termcolor color = ( piece & Piece_MaskColor ) << 4;

    if ( is_empty( piece ) || color == FGColor_BLACK )
    {
        color |= FGColor_MaskBright;
    }

    return color;
}


static struct Style generate_style( gamepiece const piece, bool const hovered )
{
    termcolor color = get_piece_color( piece );
    enum Attr const attr = /*is_future_turn( piece ) ||*/ ( piece & PieceFlag_EMPTY ) ? Attr_FAINT : Attr_NONE;

    if ( piece & PieceFlag_SECRET || piece & PieceFlag_EMPTY )
    {
        color = FGColor_BRIGHT_BLACK;
    }
    if ( hovered )
    {
        color |= FGColor_MaskBright;
    }

    return STYLE_WITH_ATTR( color, attr );
}


void piece_write_1x1( screenpos const pos, gamepiece const piece )
{
    style_update( generate_style( piece, false ) );
    cursor_update_yx( pos.y, pos.x );

    if ( is_pin( piece ) && ( ( piece & Piece_MaskAll ) == Piece_PIN_INCORRECT ) )
    {
        term_write( L" " );
        return;
    }

    if ( is_peg( piece ) && ( piece & PieceFlag_SECRET ) )
    {
        term_write( L"?" );
        return;
    }

    if ( is_empty( piece ) )
    {
        term_write( L"%lc", UTF16C_SmallDottedCircle );
        return;
    }

    utf16 const character = is_peg( piece ) ? UTF16C_BigFilledCircle : UTF16C_SmallFilledCircle;
    term_write( L"%lc", character );
}


void piece_write_4x2( screenpos const pos, gamepiece const piece )
{
    style_update( generate_style( piece, false ) );
    cursor_update_yx( pos.y, pos.x );

    if ( is_pin( piece ) && ( ( piece & Piece_MaskAll ) == Piece_PIN_INCORRECT ) )
    {
        term_write( L"    " );
	    cursor_update_yx( pos.y + 1, pos.x );
        term_write( L"    " );
        return;
    }

    if ( is_empty( piece ) )
    {
    	term_write( L".''." );
	    cursor_update_yx( pos.y + 1, pos.x );
	    term_write( L"`,,'" );
        return;
    }

	term_write( L",db." );
	cursor_update_yx( pos.y + 1, pos.x );
	term_write( L"`YP'" );
}


void piece_write_6x3( screenpos const pos, gamepiece const piece, bool hovered )
{
    style_update( generate_style( piece, hovered ) );
    cursor_update_yx( pos.y, pos.x );

    if ( is_empty( piece ) )
    {
        term_write( L",:'':." );
        cursor_update_yx( pos.y + 1, pos.x );
        term_write( L":    :" );
        cursor_update_yx( pos.y + 2, pos.x );
        term_write( L"`:,,:'" );
        return;
    }

	term_write( L",d||b." );
	cursor_update_yx( pos.y + 1, pos.x );
	term_write( L"OOOOOO" );
	cursor_update_yx( pos.y + 2, pos.x );
	term_write( L"`Y||P'" );
}
