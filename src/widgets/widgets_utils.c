#include "widgets/widgets_utils.h"

#include "console.h"

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
    BorderColor_ELLIPSIS = ConsoleColorFG_YELLOW,//BRIGHT_BLACK,
    BorderColor_TITLE = ConsoleColorFG_GREEN
};


u16 draw_optional_border_title( utf16 const *const optTitle )
{
    if ( !optTitle || optTitle[0] == L'\0' ) return 0;

    // How many spaces left ?

    console_color_fg( BorderColor_TITLE );
    u16 const titleSize = console_draw( L" %S ", optTitle );
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

    // Ellipsis calculs
    u16 const ellipsisSizeX = screenSize.x - border->upLeft.x + 1; // +1 to account the current case
    u16 const ellipsisSizeY = screenSize.y - border->upLeft.y + 1; // +1 to account the current case

    bool const ellipsisXNeeded = ellipsisSizeX < border->size.x;
    bool const ellipsisYNeeded = ellipsisSizeY < border->size.y;


    u16 const nbHorizBars = ( ellipsisXNeeded ? ellipsisSizeX - 1 : border->size.x - 2 ); // minus the 2 corners | 1 corner + 1 ellipsis
    u16 const nbVertBars  = ( ellipsisYNeeded ? ellipsisSizeY - 1 : border->size.y - 2 ); // minus 1 corner for ellipsis | 2 corners




    console_cursor_set_position( sPos.y, sPos.x );

    bool const ellipsisNeeded = ellipsisXNeeded || ellipsisYNeeded;
    console_color_fg( ellipsisNeeded ? BorderColor_ELLIPSIS : BorderColor_LINES );

    // Set the console given in the struct
    console_draw( L"%lc", Border_UP_LEFT_CORNER );

    // Ignore the title for now
    u16 const titleSize = 0;// draw_optional_border_title( border->optTitle );
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
//    if ( nbVertBars > 0 ) sPos.y += ( nbVertBars - 1 );
/*    do 
    {
    } while ( sPos.y < border->upLeft.y + nbVertBars );*/


    // Bottom borders
/*    console_cursor_set_position( sPos.y, sPos.x );
    console_draw( L"%lc", Border_BOTTOM_LEFT_CORNER );
    for ( u16 x = 0; x < nbHorizBars; ++x )
    {
        console_draw( L"%lc", Border_HORIZONTAL_BAR );
    }
    console_draw( L"%lc", Border_BOTTOM_RIGHT_CORNER );
*/
    if ( ellipsisYNeeded ) return;
    // Ellipsis attempt
    console_cursor_set_position( sPos.y + 1, sPos.x );
    console_draw( L"%lc", Border_BOTTOM_LEFT_CORNER );
    for ( u16 x = 0; x < nbHorizBars; ++x )
    {
        console_draw( L"%lc", ellipsisXNeeded ? Border_HORIZONTAL_ELLIPSIS : Border_HORIZONTAL_BAR );
    }
    if (!ellipsisXNeeded) console_draw( L"%lc", Border_BOTTOM_RIGHT_CORNER );


/*
    u32 const nbTurns = mastermind->config.nbTurns;
    u32 const nbPegsPerTurn = mastermind->config.nbCodePegPerTurn;
    // 4 -> border + space each side, + 2 -> middle with - and space
    u32 const borderWidth = 4 + 3 * nbPegsPerTurn + 2;

    // Upper border
    console_cursor_set_position( pos.y, pos.x );
    console_color_fg( ConsoleColorFG_WHITE );
    int prefixSize = console_draw( L"┌" );
    console_color_fg( ConsoleColorFG_GREEN );
    prefixSize += console_draw( L" Summary " );
    console_color_fg( ConsoleColorFG_WHITE );
    for ( int i = 0; i < borderWidth - 1 - prefixSize; ++ i )
    {
        console_draw( L"─" );
    }
    console_draw( L"┐" );
    pos.y += 1;

    // Middle border
    for ( int i = 0; i < nbTurns; i++ )
    {
        console_cursor_set_position( pos.y + i, pos.x );
        console_draw( L"│" );
        console_cursor_set_position( pos.y + i, pos.x + 2 * nbPegsPerTurn + 2 );
        console_draw( L"-" );
        console_cursor_set_position( pos.y + i, pos.x + borderWidth - 1 );
        console_draw( L"│" );
    }
    pos.y += nbTurns;

    utf16 title[] = L"MASTERMIND";
    u16 nbSpacesEachSide = ( borderWidth - ARR_COUNT( title ) ) / 2;

   	console_cursor_set_position( pos.y, pos.x );
	console_draw( L"│" );
    console_cursor_move_right_by( nbSpacesEachSide );
	console_color_fg( ConsoleColorFG_YELLOW );
	console_draw( L"MASTERMIND" );
	console_color_fg( ConsoleColorFG_WHITE );
    console_cursor_set_position( pos.y, pos.x + borderWidth - 1 );
	console_draw( L"│", nbSpacesEachSide );

    pos.y += 1;
    console_cursor_set_position( pos.y, pos.x );
	console_draw( L"│" );
    console_cursor_move_right_by( borderWidth - 2 );
	console_draw( L"│" );

    pos.y += 1;
    console_cursor_set_position( pos.y, pos.x );
    console_draw( L"└" );
    for ( int i = 0; i < borderWidth - 2; ++ i )
    {
        console_draw( L"─" );
    }
    console_draw( L"┘" );
*/
}
