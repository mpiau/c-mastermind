#include "components/component_peg_selection.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "mastermind.h"
#include "rect.h"
#include "game.h"
#include "mouse.h"

#include <stdlib.h>
#include <string.h>

struct PegButton
{
    struct Rect box;
};

struct ComponentPegSelection
{
    struct ComponentHeader header;

    enum Piece hovered;
    enum Piece selected;
    struct PegButton buttons[Piece_PEGCount];
    struct Rect box;

    struct Character dragSave[2][4];
    screenpos dragPos;
};


static void on_refresh_callback( struct ComponentHeader const *widget )
{
    struct ComponentPegSelection const *comp = (struct ComponentPegSelection const *)widget;
    rect_draw_borders( &comp->box, L"Pegs" );

    for ( usize id = 0; id < ARR_COUNT( comp->buttons ); ++id )
    {
        struct PegButton const *button = &comp->buttons[id];
        screenpos ul = rect_get_ul_corner( &button->box );
        cursor_update_pos( ul );
        if ( comp->selected == id )
        {
            style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
            term_write( L"[%u]", id );
            ul.x += 4;
            piece_write_4x2( ul, id | PieceFlag_EMPTY );
        }
        else if ( comp->hovered == id )
        {
            style_update( STYLE( FGColor_YELLOW ) );
            term_write( L"[" );
            style_update( STYLE( FGColor_BRIGHT_YELLOW ) );
            term_write( L"%u", id );
            style_update( STYLE( FGColor_YELLOW ) );
            term_write( L"]" );
            ul.x += 4;
            piece_write_4x2( ul, id );
        }
        else
        {
            style_update( STYLE( FGColor_BRIGHT_BLACK ) );
            term_write( L"[" );
            style_update( STYLE( FGColor_YELLOW ) );
            term_write( L"%u", id );
            style_update( STYLE( FGColor_BRIGHT_BLACK ) );
            term_write( L"]" );
            ul.x += 4;
            piece_write_4x2( ul, id );
        }
    }

    if ( ( comp->selected & PieceFlag_EMPTY ) == 0 )
    {
        piece_write_4x2( comp->dragPos, comp->selected );
    }
}


static void save_content_at_drag_pos( struct ComponentPegSelection *comp )
{
    for ( usize y = 0; y < 2 && comp->dragPos.y + y <= GAME_SIZE_HEIGHT; ++y )
    {
        for ( usize x = 0; x < 4 && comp->dragPos.x + x <= GAME_SIZE_WIDTH; ++x )
        {
            screenpos pos = SCREENPOS( comp->dragPos.x + x, comp->dragPos.y + y );
            comp->dragSave[y][x] = term_character_buffered_at_pos( pos );
        }
    }
}

static void write_saved_content_at_drag_pos( struct ComponentPegSelection *comp )
{
    for ( usize y = 0; y < 2 && comp->dragPos.y + y <= GAME_SIZE_HEIGHT; ++y )
    {
        for ( usize x = 0; x < 4 && comp->dragPos.x + x <= GAME_SIZE_WIDTH; ++x )
        {
            struct Character character = comp->dragSave[y][x];
            screenpos pos = SCREENPOS( comp->dragPos.x + x, comp->dragPos.y + y );

            cursor_update_pos( pos );
            style_update( character.style );
            term_write( L"%lc", character.unicode );
        }
    }    
}


static bool input_received_callback( struct ComponentHeader *widget, enum KeyInput const input )
{
    struct ComponentPegSelection *comp = (struct ComponentPegSelection *)widget;

    if ( input == KeyInput_MOUSE_BTN_LEFT && ( comp->hovered & PieceFlag_EMPTY ) == 0 )
    {
        if ( ( comp->selected & PieceFlag_EMPTY ) == 0 )
        {
            write_saved_content_at_drag_pos( comp );
        }
        screenpos pos = mouse_pos();
        pos.x -= 1;
        comp->dragPos = pos;
        save_content_at_drag_pos( comp );
        comp->selected = comp->hovered;
        widget->refreshNeeded = true;
        return true;
    }
    else if ( input == KeyInput_MOUSE_BTN_RIGHT && ( comp->selected & PieceFlag_EMPTY ) == 0 )
    {
        write_saved_content_at_drag_pos( comp );
        comp->selected = PieceFlag_EMPTY;
        widget->refreshNeeded = true;
        return true;
    }
    return false;
}


static void on_game_update_callback( struct ComponentHeader *header, enum GameUpdateType type )
{
	if ( type == GameUpdateType_GAME_NEW )
	{
		header->refreshNeeded = true;
		header->enabled = true;
	}
}


static void on_mouse_move_callback( struct ComponentHeader *header, screenpos pos )
{
	struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;

    bool set = false;
	for ( enum Piece idx = 0; idx < ARR_COUNT( comp->buttons ); ++idx )
	{
		if ( rect_is_inside( &comp->buttons[idx].box, pos ) )
		{
			if ( comp->hovered != idx )
			{
				comp->hovered = idx;
				header->refreshNeeded = true;
			}
            set = true;
			break;
		}
	}

	if ( !set && ( comp->hovered & PieceFlag_EMPTY ) == 0 )
	{
		comp->hovered = PieceFlag_EMPTY;
		header->refreshNeeded = true;
	}

    if ( comp->selected & PieceFlag_EMPTY == 0 )
    {
        write_saved_content_at_drag_pos( comp );
        comp->dragPos = mouse_pos();
        comp->dragPos.x -= 1;
        save_content_at_drag_pos( comp );
        header->refreshNeeded = true;
    }
}


struct PegButton peg_button_make( screenpos ul, vec2u16 size )
{
    return (struct PegButton) {
        .box = rect_make( ul, size )
    };
}


struct ComponentHeader *component_peg_selection_create( void )
{
    struct ComponentPegSelection *const comp = calloc( 1, sizeof( struct ComponentPegSelection ) );
    if ( !comp ) return NULL;

	component_make_header( &comp->header, ComponentId_PEG_SELECTION, false );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->refreshCb = on_refresh_callback;
    callbacks->inputReceivedCb = input_received_callback;
    callbacks->gameUpdateCb = on_game_update_callback;
    callbacks->mouseMoveCb = on_mouse_move_callback;

    // Specific to the widget 
    comp->hovered = PieceFlag_EMPTY;
    comp->selected = PieceFlag_EMPTY;

    screenpos ul = SCREENPOS( 3, 2 );
	comp->box = rect_make( ul, VEC2U16( 14, 27 ) );
    ul.x += 3;
    comp->buttons[Piece_PEG_BLACK] = peg_button_make( SCREENPOS( ul.x, ul.y + 2 ), VEC2U16( 8, 2 ) );
    comp->buttons[Piece_PEG_RED] = peg_button_make( SCREENPOS( ul.x, ul.y + 5 ), VEC2U16( 8, 2 ) );
    comp->buttons[Piece_PEG_GREEN] = peg_button_make( SCREENPOS( ul.x, ul.y + 8 ), VEC2U16( 8, 2 ) );
    comp->buttons[Piece_PEG_YELLOW] = peg_button_make( SCREENPOS( ul.x, ul.y + 11 ), VEC2U16( 8, 2 ) );
    comp->buttons[Piece_PEG_CYAN] = peg_button_make( SCREENPOS( ul.x, ul.y + 14 ), VEC2U16( 8, 2 ) );
    comp->buttons[Piece_PEG_MAGENTA] = peg_button_make( SCREENPOS( ul.x, ul.y + 17 ), VEC2U16( 8, 2 ) );
    comp->buttons[Piece_PEG_BLUE] = peg_button_make( SCREENPOS( ul.x, ul.y + 20 ), VEC2U16( 8, 2 ) );
    comp->buttons[Piece_PEG_WHITE] = peg_button_make( SCREENPOS( ul.x, ul.y + 23 ), VEC2U16( 8, 2 ) );

    comp->dragPos = SCREENPOS( 0, 0 );
    memset( comp->dragSave, 0, sizeof( comp->dragSave ) );

    return (struct ComponentHeader *)comp;
}
