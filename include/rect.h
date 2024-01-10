#pragma once

#include "core_types.h"
#include "core_unions.h"

enum RectCorner
{
	RectCorner_UL, // Up left
	RectCorner_UR, // Up Right
	RectCorner_BL, // Bottom Left
	RectCorner_BR, // Bottom Right

	RectCorner_Count
};

struct Rect
{
	screenpos_deprecated corners[RectCorner_Count];
	vec2u16 size;
};

struct Rect rect_make( screenpos_deprecated upLeft, vec2u16 size );

screenpos_deprecated rect_get_corner( struct Rect const *rect, enum RectCorner corner );
u16 rect_get_width( struct Rect const *rect );
u16 rect_get_height( struct Rect const *rect );

bool rect_check_collision( struct Rect const *lhs, struct Rect const *rhs );
bool rect_is_inside( struct Rect const *rect, screenpos_deprecated pos );

void rect_draw_borders( struct Rect const *rect, utf16 const *optTitle );
void rect_clear( struct Rect const *rect );
