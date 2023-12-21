#include "widgets/widgets_utils.h"

#include "console.h"
#include <stdio.h>

enum
{
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

enum ConsoleColorFG get_border_color( enum WidgetTruncate const truncateStatus )
{
    return truncateStatus == WidgetTruncate_NONE ? BorderColor_LINES : BorderColor_ELLIPSIS;
}

utf16 get_horiz_utf16( enum WidgetTruncate const truncateStatus )
{
    return truncateStatus == WidgetTruncate_NONE ? Border_HORIZONTAL_BAR : Border_HORIZONTAL_ELLIPSIS;
}


u16 draw_optional_border_title( utf16 const *const optTitle, u16 const maxSize, bool const isTruncated )
{
    if ( !optTitle || optTitle[0] == L'\0' ) return 0;

    utf16 title[maxSize];
    snwprintf( title, maxSize, L" %S ", optTitle );

    if ( isTruncated )
    {
        return console_draw( title );
    }

    console_color_fg( BorderColor_TITLE );
    u16 const titleSize = console_draw( title );
    console_color_fg( BorderColor_LINES );

    return titleSize;
}



void widget_utils_draw_borders( struct WidgetBorder const *border, screenpos const screenSize )
{
    // Ensure that the widget is big enough to have borders
    assert( border->size.x >= 3 && border->size.y >= 3 );

    screenpos sPos = border->upLeft;

    // Don't draw anything if upleft is out of screen
    if ( sPos.x > screenSize.x || sPos.y > screenSize.y ) return;

    enum WidgetTruncate truncate = WidgetTruncate_NONE;

    // Ellipsis calculs
    u16 const ellipsisSizeX = screenSize.x - border->upLeft.x + 1; // +1 to account the current case
    u16 const ellipsisSizeY = screenSize.y - border->upLeft.y + 1; // +1 to account the current case

    bool const ellipsisXNeeded = ellipsisSizeX < border->size.x;
    bool const ellipsisYNeeded = ellipsisSizeY < border->size.y;

    if ( ellipsisXNeeded )      truncate |= WidgetTruncate_X_AXIS;
    else if ( ellipsisYNeeded ) truncate |= WidgetTruncate_Y_AXIS;

    bool const ellipsisNeeded = ellipsisXNeeded || ellipsisYNeeded;

    u16 const nbHorizBars = ( ellipsisXNeeded ? ellipsisSizeX - 1 : border->size.x - 2 ); // minus the 2 corners | 1 corner + 1 ellipsis
    u16 const nbVertBars  = ( ellipsisYNeeded ? ellipsisSizeY - 1 : border->size.y - 2 ); // minus 1 corner for ellipsis | 2 corners


    console_cursor_set_position( sPos.y, sPos.x );

    console_color_fg( get_border_color( truncate ) );

    console_draw( L"%lc", Border_UP_LEFT_CORNER );

    // Ignore the title for now
    u16 const titleSize = draw_optional_border_title( border->optTitle, nbHorizBars, ellipsisNeeded );
    for ( u16 x = 0; x < nbHorizBars - titleSize; ++x )
    {
        console_draw( L"%lc", ellipsisXNeeded ? Border_HORIZONTAL_ELLIPSIS : Border_HORIZONTAL_BAR );
    }
    if (!ellipsisXNeeded) console_draw( L"%lc", Border_UP_RIGHT_CORNER );

    // middle part of the borders
    for ( u16 y = 0; y < nbVertBars; ++y )
    {
        sPos.y += 1;
        console_cursor_set_position( sPos.y, sPos.x );
        console_draw( L"%lc", ellipsisYNeeded ? Border_VERTICAL_ELLIPSIS : Border_VERTICAL_BAR );
        if ( !ellipsisXNeeded )
        {
            console_cursor_move_right_by( nbHorizBars );
            console_draw( L"%lc", ellipsisYNeeded ? Border_VERTICAL_ELLIPSIS : Border_VERTICAL_BAR );
        }
    }

    if ( ellipsisYNeeded ) { console_color_reset(); return; }
    // Ellipsis attempt
    console_cursor_set_position( sPos.y + 1, sPos.x );
    console_draw( L"%lc", Border_BOTTOM_LEFT_CORNER );
    for ( u16 x = 0; x < nbHorizBars; ++x )
    {
        console_draw( L"%lc", ellipsisXNeeded ? Border_HORIZONTAL_ELLIPSIS : Border_HORIZONTAL_BAR );
    }
    if (!ellipsisXNeeded) console_draw( L"%lc", Border_BOTTOM_RIGHT_CORNER );

    console_color_reset();
}
