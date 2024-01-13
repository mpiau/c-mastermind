#include "rect.h"

#include "terminal/terminal.h"
#include <stdio.h>


struct Rect rect_make( screenpos const ul, vec2u16 const size )
{
	assert( size.h > 0 && size.w > 0 );

	screenpos const br = (screenpos) {
			.x = ( ul.x + size.w ) - 1,
			.y = ( ul.y + size.h ) - 1
	};

	return (struct Rect) {
		.corners[RectCorner_UL] = ul,
		.corners[RectCorner_BR] = br,
		.size = size
	};
}


screenpos rect_get_corner( struct Rect const *rect, enum RectCorner corner )
{
	assert( rect && corner < RectCorner_Count );
	return rect->corners[corner];
}


screenpos rect_get_ul_corner( struct Rect const *rect )
{
	return rect->corners[RectCorner_UL];
}


screenpos rect_get_br_corner( struct Rect const *rect )
{
	return rect->corners[RectCorner_BR];
}


u16 rect_get_width( struct Rect const *rect )
{
	assert( rect );
	return rect->size.w;
}

u16 rect_get_height( struct Rect const *rect )
{
	assert( rect );
	return rect->size.h;
}


bool rect_check_collision( struct Rect const *lhs, struct Rect const *rhs )
{
	assert( lhs && rhs );

	screenpos const lhsUL = rect_get_ul_corner( lhs );
	screenpos const lhsBR = rect_get_br_corner( lhs );

	screenpos const rhsUL = rect_get_ul_corner( rhs );
	screenpos const rhsBR = rect_get_br_corner( rhs );

	return !( lhsUL.x > rhsBR.x // lhs is at the right of rhs
		|| lhsUL.y > rhsBR.y    // lhs is below rhs
		|| lhsBR.x < rhsUL.x    // lhs is at the left of rhs
		|| lhsBR.y < rhsUL.y    // lhs is above rhs
	); // if none of these conditions are met, they are overlapping somewhere
}


bool rect_is_inside( struct Rect const *lhs, screenpos const pos )
{
	assert( lhs );

	screenpos const lhsUL = rect_get_ul_corner( lhs );
	screenpos const lhsBR = rect_get_br_corner( lhs );

	return ( pos.x >= lhsUL.x && pos.x <= lhsBR.x )
		&& ( pos.y >= lhsUL.y && pos.y <= lhsBR.y );
}


static usize draw_optional_border_title( utf16 const *const optTitle, usize const maxSize )
{
    if ( !optTitle || optTitle[0] == L'\0' || maxSize <= 2 ) return 0;

    utf16 title[maxSize];
    snwprintf( title, maxSize, L" %S ", optTitle );

	style_update( STYLE_DEFAULT );
    usize const titleSize = term_write( title );
    style_update( STYLE( FGColor_BRIGHT_BLACK ) );

    return titleSize;
}


void rect_draw_borders( struct Rect const *rect, utf16 const *optTitle )
{
    style_update( STYLE( FGColor_BRIGHT_BLACK ) );
	
	screenpos const ul = rect_get_ul_corner( rect );
	usize const widthNoCorners = rect->size.w - 2;

	// First line
    cursor_update_pos( ul );
    term_write( L"┌" );
	usize const titleSize = draw_optional_border_title( optTitle, widthNoCorners );

    for ( usize x = 0; x < ( widthNoCorners - titleSize ); ++x )
	{
		term_write( L"─" );
	}
    term_write( L"┐" );

	// Vertical middle lines
    for ( usize y = 1; y < rect->size.h - 1; ++y )
    {
        cursor_update_yx( ul.y + y, ul.x );
        term_write( L"│" );
        cursor_move_right_by( widthNoCorners );
        term_write( L"│" );
    }

	// Last line
    cursor_update_yx( ul.y + rect->size.h - 1, ul.x );
    term_write( L"└" );
    for ( usize x = 0; x < widthNoCorners; ++x ) term_write( L"─" );
    term_write( L"┘" );
}


void rect_clear( struct Rect const *rect )
{
	screenpos const ul = rect_get_ul_corner( rect );
    for ( usize y = 0; y < rect->size.h; ++y )
    {
        cursor_update_yx( ul.y + y, ul.x );
		term_write( L"%*lc", rect->size.w, L' ' );
    }
}
