#include "rect.h"

#include "console.h"
#include <stdio.h>

struct Rect rect_make( screenpos_deprecated const ul, vec2u16 const size )
{
	assert( size.h > 0 && size.w > 0 );

	screenpos_deprecated const ur = (screenpos_deprecated) {
			.x = ( ul.x + size.w ) - 1,
			.y = ul.y
	};
	screenpos_deprecated const bl = (screenpos_deprecated) {
			.x = ul.x,
			.y = ( ul.y + size.h ) - 1
	};
	screenpos_deprecated const br = (screenpos_deprecated) {
			.x = ur.x,
			.y = bl.y
	};

	return (struct Rect) {
		.corners[RectCorner_UL] = ul,
		.corners[RectCorner_UR] = ur,
		.corners[RectCorner_BL] = bl,
		.corners[RectCorner_BR] = br,
		.size = size
	};
}


screenpos_deprecated rect_get_corner( struct Rect const *rect, enum RectCorner corner )
{
	assert( rect && corner < RectCorner_Count );
	return rect->corners[corner];
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

	screenpos_deprecated const lhsUL = rect_get_corner( lhs, RectCorner_UL );
	screenpos_deprecated const lhsBR = rect_get_corner( lhs, RectCorner_BR );

	screenpos_deprecated const rhsUL = rect_get_corner( rhs, RectCorner_UL );
	screenpos_deprecated const rhsBR = rect_get_corner( rhs, RectCorner_BR );

	return !( lhsUL.x > rhsBR.x // lhs is at the right of rhs
		|| lhsUL.y > rhsBR.y // lhs is below rhs
		|| lhsBR.x < rhsUL.x // lhs is at the left of rhs
		|| lhsBR.y < rhsUL.y // lhs is above rhs
	); // if none of these conditions are met, they are overlapping somewhere
}


bool rect_is_inside( struct Rect const *lhs, screenpos_deprecated const pos )
{
	assert( lhs );

	screenpos_deprecated const lhsUL = rect_get_corner( lhs, RectCorner_UL );
	screenpos_deprecated const lhsBR = rect_get_corner( lhs, RectCorner_BR );

	return ( pos.x >= lhsUL.x && pos.x <= lhsBR.x )
		&& ( pos.y >= lhsUL.y && pos.y <= lhsBR.y );
}


static usize draw_optional_border_title( utf16 const *const optTitle, usize const maxSize )
{
    if ( !optTitle || optTitle[0] == L'\0' || maxSize <= 2 ) return 0;

    utf16 title[maxSize];
    snwprintf( title, maxSize, L" %S ", optTitle );

    console_color_fg( ConsoleColorFG_WHITE );
    usize const titleSize = console_draw( title );
    console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

    return titleSize;
}


void rect_draw_borders( struct Rect const *rect, utf16 const *optTitle )
{
    console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
	
	screenpos_deprecated const ul = rect_get_corner( rect, RectCorner_UL );
	usize const widthNoCorners = rect->size.w - 2;

	// First line
    console_setpos( ul );
    console_draw( L"┌" );
	usize const titleSize = draw_optional_border_title( optTitle, widthNoCorners );

    for ( usize x = 0; x < ( widthNoCorners - titleSize ); ++x )
	{
		console_draw( L"─" );
	}
    console_draw( L"┐" );

	// Vertical middle lines
    for ( usize y = 1; y < rect->size.h - 1; ++y )
    {
        console_cursor_set_position( ul.y + y, ul.x );
        console_draw( L"│" );
        console_cursor_move_right_by( widthNoCorners );
        console_draw( L"│" );
    }

	// Last line
    console_cursor_set_position( ul.y + rect->size.h - 1, ul.x );
    console_draw( L"└" );
    for ( usize x = 0; x < widthNoCorners; ++x ) console_draw( L"─" );
    console_draw( L"┘" );
}


void rect_clear( struct Rect const *rect )
{
	screenpos_deprecated const ul = rect_get_corner( rect, RectCorner_UL );
    for ( usize y = 0; y < rect->size.h; ++y )
    {
        console_cursor_set_position( ul.y + y, ul.x );
		console_draw( L"%*lc", rect->size.w, L' ' );
    }
}
