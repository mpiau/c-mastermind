#include "widgets/widget_utils.h"

#include "console.h"
#include "console_screen.h"
#include <stdio.h>

enum
{
	// TODO replace by characters_list.h
    Border_UP_LEFT_CORNER = L'┌',
    Border_UP_RIGHT_CORNER = L'┐',
    Border_BOTTOM_LEFT_CORNER = L'└',
    Border_BOTTOM_RIGHT_CORNER = L'┘',
    Border_HORIZONTAL_BAR = L'─',
    Border_HORIZONTAL_ELLIPSIS = L'╌',
    Border_VERTICAL_BAR = L'│',
    Border_VERTICAL_ELLIPSIS = L'┆',


    BorderColor_LINES = ConsoleColorFG_WHITE,
    BorderColor_ELLIPSIS = ConsoleColorFG_YELLOW,
    BorderColor_TITLE = ConsoleColorFG_GREEN
};

enum ConsoleColorFG get_border_color( enum WidgetTruncatedStatus const truncateStatus )
{
    return truncateStatus == WidgetTruncatedStatus_NONE ? BorderColor_LINES : BorderColor_ELLIPSIS;
}

utf16 get_horiz_utf16( enum WidgetTruncatedStatus const truncateStatus )
{
    return truncateStatus == WidgetTruncatedStatus_NONE ? Border_HORIZONTAL_BAR : Border_HORIZONTAL_ELLIPSIS;
}


u16 draw_optional_border_title( utf16 const *const optTitle, u16 const maxSize, bool const isTruncated, enum ConsoleColorFG color )
{
    if ( !optTitle || optTitle[0] == L'\0' ) return 0;

    utf16 title[maxSize];
    snwprintf( title, maxSize, L" %S ", optTitle );

    if ( isTruncated )
    {
        return console_draw( title );
    }

    console_color_fg( color );
    u16 const titleSize = console_draw( title );
    console_color_fg( color );

    return titleSize;
}



void widget_utils_draw_borders( struct WidgetBox const *box )
{
    screenpos borderUL = box->borderUpLeft;
	if ( box->truncatedStatus == WidgetTruncatedStatus_OUT_OF_BOUNDS ) return;

    bool const ellipsisNeeded = box->truncatedStatus != WidgetTruncatedStatus_NONE;
	bool const ellipsisXNeeded = ( box->truncatedStatus & WidgetTruncatedStatus_X_AXIS ) != 0;
	bool const ellipsisYNeeded = ( box->truncatedStatus & WidgetTruncatedStatus_Y_AXIS ) != 0;
	screenpos const borderBR = ellipsisNeeded ? box->truncatedBorderBottomRight : box->borderBottomRight;
	u16 const width = borderBR.x - borderUL.x + 1;
	u16 const height = borderBR.y - borderUL.y + 1;

    u16 const nbHorizBars = ellipsisXNeeded ? width - 1 : width - 2;
    u16 const nbVertBars  = ellipsisYNeeded ? height - 1 : height - 2;

    console_cursor_set_position( borderUL.y, borderUL.x );

    console_color_fg( ellipsisNeeded ? BorderColor_ELLIPSIS : box->borderColor );

    console_draw( L"%lc", Border_UP_LEFT_CORNER );

    // Ignore the title for now
    u16 const titleSize = draw_optional_border_title( box->borderOptionalTitle, nbHorizBars, ellipsisNeeded, box->borderTitleColor );
	console_color_fg( ellipsisNeeded ? BorderColor_ELLIPSIS : box->borderColor );
    for ( u16 x = 0; x < nbHorizBars - titleSize; ++x )
    {
        console_draw( L"%lc", ellipsisXNeeded ? Border_HORIZONTAL_ELLIPSIS : Border_HORIZONTAL_BAR );
    }
    if (!ellipsisXNeeded) console_draw( L"%lc", Border_UP_RIGHT_CORNER );

    // middle part of the borders
    for ( u16 y = 0; y < nbVertBars; ++y )
    {
        borderUL.y += 1;
        console_cursor_set_position( borderUL.y, borderUL.x );
        console_draw( L"%lc", ellipsisYNeeded ? Border_VERTICAL_ELLIPSIS : Border_VERTICAL_BAR );
        if ( !ellipsisXNeeded )
        {
            console_cursor_move_right_by( nbHorizBars );
            console_draw( L"%lc", ellipsisYNeeded ? Border_VERTICAL_ELLIPSIS : Border_VERTICAL_BAR );
        }
    }

    if ( ellipsisYNeeded ) { console_color_reset(); return; }
    // Ellipsis attempt
    console_cursor_set_position( borderUL.y + 1, borderUL.x );
    console_draw( L"%lc", Border_BOTTOM_LEFT_CORNER );
    for ( u16 x = 0; x < nbHorizBars; ++x )
    {
        console_draw( L"%lc", ellipsisXNeeded ? Border_HORIZONTAL_ELLIPSIS : Border_HORIZONTAL_BAR );
    }
    if (!ellipsisXNeeded) console_draw( L"%lc", Border_BOTTOM_RIGHT_CORNER );

    console_color_reset();
}


void widget_utils_clear_content( struct WidgetBox *box )
{
/*	screenpos contentUL = box->contentUpLeft.x;
	for ( u16 y = 0; y < border->size.y - 2; ++y )
	{
		console_cursor_set_position( border->upLeft.y + 1 + y, border->upLeft.x + 1 ); // + 1 for borders
		console_draw( L"%*lc", border->size.x - 2, L' ' );
	}*/
}


void widget_utils_set_title( struct WidgetBox *const box, utf16 const *const title, enum ConsoleColorFG const color )
{
	assert( box );
	assert( title );

	box->borderOptionalTitle = title;
	box->borderTitleColor = color;
}


void widget_utils_calculate_truncation( struct WidgetBox *const box, screenpos const screenSize )
{
	screenpos const borderUL = box->borderUpLeft;
	screenpos const borderBR = box->borderBottomRight;

	// Edge cases where it's not necessary to calculate truncation
	if ( borderBR.x <= screenSize.x && borderBR.y <= screenSize.y )
	{
		box->truncatedStatus = WidgetTruncatedStatus_NONE;
		return; // Everything normal here, no truncation / out-of-bounds
	}
	else if ( borderUL.x > screenSize.x && borderUL.y > screenSize.y )
	{
		box->truncatedStatus = WidgetTruncatedStatus_OUT_OF_BOUNDS;
		return;
	}

	box->truncatedBorderBottomRight = borderBR;

	screenpos const truncBorderBR = (screenpos) {
		.x = screenSize.x - borderUL.x + 1,
		.y = screenSize.y - borderUL.y + 1
	};

	box->truncatedStatus = WidgetTruncatedStatus_NONE;
	if ( truncBorderBR.x < borderBR.x )
	{
		box->truncatedBorderBottomRight.x = truncBorderBR.x;
		box->truncatedStatus |= WidgetTruncatedStatus_X_AXIS;
	}
	if ( truncBorderBR.y < borderBR.y )
	{
		box->truncatedBorderBottomRight.y = truncBorderBR.y;
		box->truncatedStatus |= WidgetTruncatedStatus_Y_AXIS;
	}
}


void widget_utils_set_position( struct WidgetBox *const box, screenpos const borderUpLeft, vec2u16 const contentSize )
{
	// Note : Do some validation on the data before ?

	box->borderUpLeft = borderUpLeft;
	box->contentUpLeft = (vec2u16) {
		.x = borderUpLeft.x + 1,
		.y = borderUpLeft.y + 1
	};

	box->contentBottomRight = (vec2u16) {
		.x = box->contentUpLeft.x + ( contentSize.x - 1 ),
		.y = box->contentUpLeft.y + ( contentSize.y - 1 )
	};

	box->borderBottomRight = (vec2u16) {
		.x = box->contentBottomRight.x + 1,
		.y = box->contentBottomRight.y + 1
	};

	widget_utils_calculate_truncation( box, console_screen_get_size() );
}