#include "widgets/widget_border.h"
#include "widgets/widget_definition.h"

bool widget_is_truncated( struct WidgetBox const *box )
{
	return box->truncatedStatus == WidgetTruncatedStatus_X_AXIS || box->truncatedStatus == WidgetTruncatedStatus_Y_AXIS;
}

bool widget_is_out_of_bounds( struct WidgetBox const *box )
{
	return box->truncatedStatus == WidgetTruncatedStatus_OUT_OF_BOUNDS;
}

vec2u16 widget_border_get_size( struct WidgetBox const *box )
{
	screenpos const borderBR = widget_is_truncated( box ) ? box->truncatedBorderBottomRight : box->borderBottomRight;
	return (vec2u16) {
		.x = borderBR.x - box->borderUpLeft.x + 1,
		.y = borderBR.y - box->borderUpLeft.y + 1
	};
}


vec2u16 widget_content_get_size( struct WidgetBox const *box )
{
	screenpos const contentBR = widget_is_truncated( box ) ? box->truncatedContentBottomRight : box->contentBottomRight;
	return (vec2u16) {
		.x = contentBR.x - box->contentUpLeft.x + 1,
		.y = contentBR.y - box->contentUpLeft.y + 1
	};	
}