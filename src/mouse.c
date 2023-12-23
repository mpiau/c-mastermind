#include "mouse.h"

static screenpos s_mousePos = {};

screenpos mouse_get_position()
{
	return s_mousePos;
}

void mouse_on_position_update( vec2u16 const pos )
{
	s_mousePos = (screenpos) {
		.x = pos.x + 1,
		.y = pos.y + 1
	};
}
