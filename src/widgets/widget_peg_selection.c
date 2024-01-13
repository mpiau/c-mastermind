#include "widgets/widget_peg_selection.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "mastermind.h"
#include "rect.h"

#include <stdlib.h>
#include <string.h>


struct WidgetPegSelection
{
    struct ComponentHeader header;

    enum PegId selection;
    struct Rect box;
};


static void draw_colored_peg( struct ComponentHeader const *widget, screenpos const ul, enum PegId const id )
{
    struct WidgetPegSelection const *selection = (struct WidgetPegSelection const *)widget;
	style_update( STYLE( peg_get_color( id, selection->selection == id ) ) );

	cursor_update_yx( ul.y, ul.x );
	term_write( L",db." );
	cursor_update_yx( ul.y + 1, ul.x );
	term_write( L"`YP'" );
}

static void draw_peg( screenpos const ul, enum PegId const id )
{
	bool const isEmpty = ( id == PegId_Empty );

	style_update( STYLE( peg_get_color( id, false ) ) );
	cursor_update_yx( ul.y, ul.x );
	term_write( isEmpty ? L",:'':." : L",d||b." );
	cursor_update_yx( ul.y + 1, ul.x );
	term_write( isEmpty ? L":    :" : L"OOOOOO" );
	cursor_update_yx( ul.y + 2, ul.x );
	term_write( isEmpty ? L"`:,,:'" : L"`Y||P'" );
}


static void draw_rect_borders( struct Rect const *rect )
{
    style_update( STYLE( FGColor_BRIGHT_BLUE ) );
    screenpos const ul = rect_get_ul_corner( rect );
    cursor_update_pos( ul );

    term_write( L"┌" );
    for ( u16 x = 0; x < rect->size.w - 2; ++x ) term_write( L"─" );
    term_write( L"┐" );

    for ( u16 y = 1; y < rect->size.h - 1; ++y )
    {
        cursor_update_yx( ul.y + y, ul.x );
        term_write( L"│" );
        cursor_move_right_by( rect->size.w - 2 );
        term_write( L"│" );
    }

    cursor_update_yx( ul.y + rect->size.h - 1, ul.x );
    term_write( L"└" );
    for ( u16 x = 0; x < rect->size.w - 2; ++x ) term_write( L"─" );
    term_write( L"┘" );
}


static void on_refresh_callback( struct ComponentHeader const *widget )
{
    struct WidgetPegSelection const *selection = (struct WidgetPegSelection const *)widget;
    screenpos ul = rect_get_ul_corner( &selection->box );
    draw_rect_borders( &selection->box );
    ul.y += 1;
    ul.x += 2;

    usize const totalWidth = selection->box.size.w;
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
    style_update( STYLE( FGColor_BRIGHT_BLUE ) );
    screenpos const vertLineUpPos = SCREENPOS( ul.x + rowPegWidth + 1, ul.y );

    cursor_update_yx( vertLineUpPos.y - 1, vertLineUpPos.x );
    term_write( L"┬" );
    for ( int y = 0; y < 5; ++y )
    {
        cursor_update_yx( vertLineUpPos.y + y, vertLineUpPos.x );
        term_write( L"%lc", L'│' );
    }
    cursor_update_yx( vertLineUpPos.y + 5, vertLineUpPos.x );
    term_write( L"┴" );

    ul.x += rowPegWidth + 3;
    ul.y += 1;

    draw_peg( ul, ((struct WidgetPegSelection *)widget)->selection );
}


static bool input_received_callback( struct ComponentHeader *widget, enum KeyInput const input )
{
    // TEMP, need the keyInput to test something
    return false;

    struct WidgetPegSelection *pegSelect = (struct WidgetPegSelection *)widget;

    switch ( input )
    {
        case KeyInput_NUMPAD_8:
        {
            if ( pegSelect->selection >= 4 )
            {
                pegSelect->selection -= 4;
                widget->refreshNeeded = true;
            }
            break;
        }
        case KeyInput_NUMPAD_2:
        {
            if ( pegSelect->selection < 4 )
            {
                pegSelect->selection += 4;
                widget->refreshNeeded = true;
            }
            break;
        }
        case KeyInput_NUMPAD_4:
        {
            if ( pegSelect->selection != 0 && pegSelect->selection != 4 )
            {
                pegSelect->selection -= 1;
                widget->refreshNeeded = true;
            }
            break;
        }
        case KeyInput_NUMPAD_6:
        {
            if ( pegSelect->selection != 3 && pegSelect->selection != 7 )
            {
                pegSelect->selection += 1;
                widget->refreshNeeded = true;
            }
            break;
        }
        default: return false;
    }

    return true;
}

struct ComponentHeader *widget_peg_selection_create( void )
{
    struct WidgetPegSelection *const pegSelection = malloc( sizeof( struct WidgetPegSelection ) );
    if ( !pegSelection ) return NULL;
    memset( pegSelection, 0, sizeof( *pegSelection ) );

    struct ComponentHeader *const widget = &pegSelection->header;
    widget->id = ComponentId_PEG_SELECTION;
    widget->enabled = false;
	widget->refreshNeeded = false;

	pegSelection->box = rect_make( SCREENPOS( 87, 21 ), VEC2U16( 32, 7 ) );

    struct ComponentCallbacks *const callbacks = &widget->callbacks;
    callbacks->refreshCb = on_refresh_callback;
    callbacks->inputReceivedCb = input_received_callback;

    // Specific to the widget 

    pegSelection->selection = PegId_RED;

    return (struct ComponentHeader *)pegSelection;
}
