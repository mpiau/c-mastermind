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
*/

static inline bool is_peg( struct Piece const piece )
{
    return piece.type == PieceType_PEG;
}

static inline bool is_pin( struct Piece const piece )
{
    return piece.type == PieceType_PIN;
}


static struct Style peg_style( struct Peg const peg )
{
    if ( peg.hidden ) return STYLE( FGColor_BRIGHT_BLACK );

    switch( peg.id )
    {
        case PegId_BLACK:   return STYLE( FGColor_BRIGHT_BLACK );
        case PegId_RED:     return STYLE( FGColor_RED );
        case PegId_GREEN:   return STYLE( FGColor_GREEN );
        case PegId_YELLOW:  return STYLE( FGColor_YELLOW );
        case PegId_BLUE:    return STYLE( FGColor_BLUE );
        case PegId_MAGENTA: return STYLE( FGColor_MAGENTA );
        case PegId_CYAN:    return STYLE( FGColor_CYAN );
        case PegId_WHITE:   return STYLE( FGColor_WHITE );

        case PegId_EMPTY:   
        default:            return STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
    }
}


static struct Style pin_style( struct Pin const pin )
{
    switch( pin.id )
    {
        case PinId_CORRECT:   return STYLE( FGColor_RED );
        case PinId_PARTIAL:   return STYLE( FGColor_WHITE );

        case PinId_INCORRECT:
        default:              return STYLE( FGColor_BLACK );
    }
}


void pin_write_1x1( screenpos const pos, struct Pin const pin )
{
    style_update( pin_style( pin ) );
    cursor_update_yx( pos.y, pos.x );

    if ( pin.id == PinId_INCORRECT )
    {
        term_write( L" " );
        return;
    }
    term_write( L"%lc", UTF16C_SmallFilledCircle );
}


void pin_write_4x2( screenpos const pos, struct Pin const pin )
{
    style_update( pin_style( pin ) );
    cursor_update_yx( pos.y, pos.x );

    if ( pin.id == PinId_INCORRECT )
    {
        term_write( L"    " );
        cursor_update_yx( pos.y + 1, pos.x );
        term_write( L"    " );
        return;
    }

	term_write( L",db." );
	cursor_update_yx( pos.y + 1, pos.x );
	term_write( L"`YP'" );    
}


void pin_write_6x3( screenpos const pos, struct Pin const pin, bool const hovered )
{
    style_update( pin_style( pin ) );
    cursor_update_yx( pos.y, pos.x );

    if ( pin.id == PinId_INCORRECT )
    {
        term_write( L"      " );
        cursor_update_yx( pos.y + 1, pos.x );
        term_write( L"      " );
        cursor_update_yx( pos.y + 2, pos.x );
        term_write( L"      " );
        return;
    }

	term_write( L",d||b." );
	cursor_update_yx( pos.y + 1, pos.x );
	term_write( L"OOOOOO" );
	cursor_update_yx( pos.y + 2, pos.x );
	term_write( L"`Y||P'" );
}


void peg_write_1x1( screenpos const pos, struct Peg const peg )
{
    style_update( peg_style( peg ) );
    cursor_update_yx( pos.y, pos.x );

    if ( peg.hidden )
    {
        term_write( L"?" );
        return;
    }
    else if ( peg.id == PegId_EMPTY )
    {
        term_write( L"%lc", UTF16C_SmallDottedCircle );
        return;
    }

    term_write( L"%lc", UTF16C_BigFilledCircle );
}


void peg_write_4x2( screenpos const pos, struct Peg const peg )
{
    style_update( peg_style( peg ) );
    cursor_update_yx( pos.y, pos.x );

    if ( peg.id == PegId_EMPTY )
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


void peg_write_6x3( screenpos const pos, struct Peg const peg, bool const hovered )
{
    style_update( peg_style( peg ) );
    cursor_update_yx( pos.y, pos.x );

    if ( peg.id == PegId_EMPTY )
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


void piece_write_1x1( screenpos const pos, struct Piece const piece )
{
    if ( is_peg( piece ) )
    {
        peg_write_1x1( pos, piece.peg );
    }
    else
    {
        pin_write_1x1( pos, piece.pin );
    }
}


void piece_write_4x2( screenpos const pos, struct Piece const piece )
{
    if ( is_peg( piece ) )
    {
        peg_write_4x2( pos, piece.peg );
    }
    else
    {
        pin_write_4x2( pos, piece.pin );
    }
}


void piece_write_6x3( screenpos const pos, struct Piece const piece, bool const hovered )
{
    if ( is_peg( piece ) )
    {
        peg_write_6x3( pos, piece.peg, hovered );
    }
    else
    {
        pin_write_6x3( pos, piece.pin, hovered );
    }
}
