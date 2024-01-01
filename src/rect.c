#include "rect.h"

struct Rect rect_make( screenpos const ul, vec2u16 const size )
{
	assert( size.h > 0 && size.w > 0 );

	screenpos const ur = (screenpos) {
			.x = ( ul.x + size.w ) - 1,
			.y = ul.y
	};
	screenpos const bl = (screenpos) {
			.x = ul.x,
			.y = ( ul.y + size.h ) - 1
	};
	screenpos const br = (screenpos) {
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


screenpos rect_get_corner( struct Rect const *rect, enum RectCorner corner )
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

	screenpos const lhsUL = rect_get_corner( lhs, RectCorner_UL );
	screenpos const lhsBR = rect_get_corner( lhs, RectCorner_BR );

	screenpos const rhsUL = rect_get_corner( rhs, RectCorner_UL );
	screenpos const rhsBR = rect_get_corner( rhs, RectCorner_BR );

	return !( lhsUL.x > rhsBR.x // lhs is at the right of rhs
		|| lhsUL.y > rhsBR.y // lhs is below rhs
		|| lhsBR.x < rhsUL.x // lhs is at the left of rhs
		|| lhsBR.y < rhsUL.y // lhs is above rhs
	); // if none of these conditions are met, they are overlapping somewhere
}


bool rect_is_inside( struct Rect const *lhs, screenpos const pos )
{
	assert( lhs );

	screenpos const lhsUL = rect_get_corner( lhs, RectCorner_UL );
	screenpos const lhsBR = rect_get_corner( lhs, RectCorner_BR );

	return ( pos.x >= lhsUL.x && pos.x <= lhsBR.x )
		&& ( pos.y >= lhsUL.y && pos.y <= lhsBR.y );
}
