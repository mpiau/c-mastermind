#pragma once

#include "core/core.h"

enum RectCorner
{
	RectCorner_UL, // Up left
	RectCorner_BR, // Bottom Right

	RectCorner_Count
};

struct Rect
{
	screenpos corners[RectCorner_Count];
	vec2u16 size;
};

struct Rect rect_make( screenpos upLeft, vec2u16 size );

screenpos rect_get_corner( struct Rect const *rect, enum RectCorner corner );
screenpos rect_get_ul_corner( struct Rect const *rect );
screenpos rect_get_br_corner( struct Rect const *rect );
u16 rect_get_width( struct Rect const *rect );
u16 rect_get_height( struct Rect const *rect );

bool rect_check_collision( struct Rect const *lhs, struct Rect const *rhs );
bool rect_is_inside( struct Rect const *rect, screenpos pos );

void rect_draw_borders( struct Rect const *rect, utf16 const *optTitle );
void rect_clear( struct Rect const *rect );
void rect_clear_content( struct Rect const *rect );
void rect_clear_borders( struct Rect const *rect );