#include "widgets/widget_peg_selection.h"

#include "widgets/widget_definition.h"

struct WidgetPegSelection
{
    struct Widget header;

    enum PegId selection;
};


static void draw_colored_peg( struct Widget *widget, screenpos const ul, enum PegId const id )
{
    struct WidgetPegSelection *selection = (struct WidgetPegSelection *)widget;
	console_color_fg( peg_get_color( id, selection->selection == id ) );

	console_cursor_set_position( ul.y, ul.x );
	console_draw( L",db." );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L"`YP'" );
}

static void draw_peg( screenpos const ul, enum PegId const id )
{
	bool const isEmpty = ( id == PegId_Empty );

	console_color_fg( peg_get_color( id, false ) );
	console_cursor_set_position( ul.y, ul.x );
	console_draw( isEmpty ? L",:'':." : L",d||b." );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( isEmpty ? L":    :" : L"OOOOOO" );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( isEmpty ? L"`:,,:'" : L"`Y||P'" );
}


static void draw_rect_borders( struct Rect *rect )
{
    console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
    screenpos const ul = rect_get_corner( rect, RectCorner_UL );
    console_setpos( ul );

    console_draw( L"┌" );
    for ( u16 x = 0; x < rect->size.w - 2; ++x ) console_draw( L"─" );
    console_draw( L"┐" );

    for ( u16 y = 1; y < rect->size.h - 1; ++y )
    {
        console_cursor_set_position( ul.y + y, ul.x );
        console_draw( L"│" );
        console_cursor_move_right_by( rect->size.w - 2 );
        console_draw( L"│" );
    }

    console_cursor_set_position( ul.y + rect->size.h - 1, ul.x );
    console_draw( L"└" );
    for ( u16 x = 0; x < rect->size.w - 2; ++x ) console_draw( L"─" );
    console_draw( L"┘" );
}


static void redraw_callback( struct Widget *widget )
{
    screenpos ul = rect_get_corner( &widget->rectBox, RectCorner_UL );
    draw_rect_borders( &widget->rectBox );
    ul.y += 1;
    ul.x += 2;

    usize const totalWidth = widget->rectBox.size.w;
    usize const rowPegWidth = 4 * 4 + 3;

    // Row lines
    for ( int y = 0; y < 2; ++y )
    {
        for ( int x = 0; x < 4; ++x )
        {
            screenpos pos = SCREENPOS( ul.x + x * 5, ul.y + y * 3 );
            enum PegId const id = y * 4 + x;
            assert( id < PegId_Count );
            draw_colored_peg( widget, pos, id );
        }
    }

    // Vertical line for separation 
    console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
    screenpos const vertLineUpPos = SCREENPOS( ul.x + rowPegWidth + 1, ul.y );

    console_cursor_set_position( vertLineUpPos.y - 1, vertLineUpPos.x );
    console_draw( L"┬" );
    for ( int y = 0; y < 5; ++y )
    {
        console_cursor_set_position( vertLineUpPos.y + y, vertLineUpPos.x );
        console_draw( L"%lc", L'│' );
    }
    console_cursor_set_position( vertLineUpPos.y + 5, vertLineUpPos.x );
    console_draw( L"┴" );

    ul.x += rowPegWidth + 3;
    ul.y += 1;

    draw_peg( ul, ((struct WidgetPegSelection *)widget)->selection );
}


static bool input_received_callback( struct Widget *widget, enum KeyInput const input )
{
    struct WidgetPegSelection *pegSelect = (struct WidgetPegSelection *)widget;

    switch ( input )
    {
        case KeyInput_NUMPAD_8:
        {
            if ( pegSelect->selection >= 4 )
            {
                pegSelect->selection -= 4;
                widget->redrawNeeded = true;
            }
            break;
        }
        case KeyInput_NUMPAD_2:
        {
            if ( pegSelect->selection < 4 )
            {
                pegSelect->selection += 4;
                widget->redrawNeeded = true;
            }
            break;
        }
        case KeyInput_NUMPAD_4:
        {
            if ( pegSelect->selection != 0 && pegSelect->selection != 4 )
            {
                pegSelect->selection -= 1;
                widget->redrawNeeded = true;
            }
            break;
        }
        case KeyInput_NUMPAD_6:
        {
            if ( pegSelect->selection != 3 && pegSelect->selection != 7 )
            {
                pegSelect->selection += 1;
                widget->redrawNeeded = true;
            }
            break;
        }
        default: return false;
    }

    return true;
}

struct Widget *widget_peg_selection_create( void )
{
    struct WidgetPegSelection *const pegSelection = malloc( sizeof( struct WidgetPegSelection ) );
    if ( !pegSelection ) return NULL;
    memset( pegSelection, 0, sizeof( *pegSelection ) );

    struct Widget *const widget = &pegSelection->header;
    widget->id = WidgetId_PEG_SELECTION;
    widget->enabled = false;
	widget->redrawNeeded = false;

	widget->rectBox = rect_make( SCREENPOS( 87, 21 ), VEC2U16( 32, 7 ) );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
    callbacks->redrawCb = redraw_callback;
    callbacks->inputReceivedCb = input_received_callback;

    // Specific to the widget 

    pegSelection->selection = PegId_RED;

    return (struct Widget *)pegSelection;
}
