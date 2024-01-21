#include "components/component_peg_selection.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "mastermind.h"
#include "rect.h"
#include "game.h"
#include "mouse.h"
#include "button.h"

#include <stdlib.h>
#include <string.h>

struct ComponentPegSelection
{
    struct ComponentHeader header;

    enum Piece hovered;
    enum Piece selected;
    struct Button buttons[Piece_PEGCount];
    struct Rect box;

    struct Character dragSave[2][4];
    screenpos dragPos;
};


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


static void write_dragged_peg( struct ComponentPegSelection *comp )
{
    if ( comp->selected != PieceFlag_EMPTY )
    {
        piece_write_4x2( comp->dragPos, comp->selected );
    }   
}


static void write_peg_button( struct Button const *button, gamepiece const piece, bool const isHovered )
{
    button_write( button, isHovered );

    screenpos ul = rect_get_ul_corner( &button->rect );
    ul.x += 6;
    piece_write_4x2( ul, piece );
}


static bool input_received_callback( struct ComponentHeader *header, enum KeyInput const input )
{
    struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;

    if ( input == KeyInput_MOUSE_BTN_LEFT && comp->hovered != PieceFlag_EMPTY )
    {
        if ( comp->selected != PieceFlag_EMPTY )
        {
            write_saved_content_at_drag_pos( comp );
        }
        screenpos pos = mouse_pos();
        pos.x -= 1;
        comp->dragPos = pos;
        save_content_at_drag_pos( comp );
        comp->selected = comp->hovered;
        write_dragged_peg( comp );
        enum Piece idx = comp->selected & Piece_MaskColor;
        button_disable( &comp->buttons[idx] );
        button_write( &comp->buttons[idx], false );
        return true;
    }
    else if ( input == KeyInput_MOUSE_BTN_RIGHT && comp->selected != PieceFlag_EMPTY )
    {
        enum Piece idx = ( comp->selected & Piece_MaskColor );
        button_enable( &comp->buttons[idx] );
        button_write( &comp->buttons[idx], false );
        write_saved_content_at_drag_pos( comp );
        comp->selected = PieceFlag_EMPTY;
        write_dragged_peg( comp );
        return true;
    }
    return false;
}


static void on_game_update_callback( struct ComponentHeader *header, enum GameUpdateType type )
{
}


static void on_mouse_move_callback( struct ComponentHeader *header, screenpos pos )
{
	struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;

    bool set = false;
	for ( enum Piece idx = 0; idx < ARR_COUNT( comp->buttons ); ++idx )
	{
        struct Button *button = &comp->buttons[idx];
		if ( button_is_hovered_by( button, pos ) )
		{
			if ( comp->hovered != idx )
			{
				comp->hovered = idx;
                write_peg_button( button, idx, true );
			}
            set = true;
			break;
		}
	}

	if ( !set && comp->hovered != PieceFlag_EMPTY )
	{
        enum Piece const idx = ( comp->hovered & Piece_MaskColor );
        write_peg_button( &comp->buttons[idx], idx, false );
		comp->hovered = PieceFlag_EMPTY;
	}

    if ( comp->selected != PieceFlag_EMPTY )
    {
        write_saved_content_at_drag_pos( comp );
        comp->dragPos = mouse_pos();
        comp->dragPos.x -= 1;
        save_content_at_drag_pos( comp );
        write_dragged_peg( comp );
    }
}


static void enable_callback( struct ComponentHeader *header )
{
    struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;
    rect_draw_borders( &comp->box, L"Pegs" );
    screenpos const mousePos = mouse_pos();
    for ( enum Piece idx = 0; idx < ARR_COUNT( comp->buttons ); ++idx )
    {
        struct Button const *button = &comp->buttons[idx];
        bool const hovered = button_is_hovered_by( button, mousePos );
        write_peg_button( button, idx, hovered );
    }
}


static void disable_callback( struct ComponentHeader *header )
{
    struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;
    rect_clear( &comp->box );
}


struct ComponentHeader *component_peg_selection_create( void )
{
    struct ComponentPegSelection *const comp = calloc( 1, sizeof( struct ComponentPegSelection ) );
    if ( !comp ) return NULL;

    comp->header.id = ComponentId_PEG_SELECTION;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->enableCb = enable_callback;
    callbacks->disableCb = disable_callback;   
    callbacks->inputReceivedCb = input_received_callback;
    callbacks->gameUpdateCb = on_game_update_callback;
    callbacks->mouseMoveCb = on_mouse_move_callback;

    // Specific to the header 
    comp->hovered = PieceFlag_EMPTY;
    comp->selected = PieceFlag_EMPTY;

    screenpos ul = SCREENPOS( 3, 2 );
    vec2u16 const size = VEC2U16( 10, 2 );
	comp->box = rect_make( ul, VEC2U16( 14, 27 ) );
    ul.x += 2;
    comp->buttons[Piece_PEG_BLACK]   = button_make( L"Grey", SCREENPOS( ul.x, ul.y + 2 ), size, KeyInput_0, true );
    comp->buttons[Piece_PEG_RED]     = button_make( L"Red", SCREENPOS( ul.x, ul.y + 5 ), size, KeyInput_1, true );
    comp->buttons[Piece_PEG_GREEN]   = button_make( L"Green", SCREENPOS( ul.x, ul.y + 8 ), size, KeyInput_2, true );
    comp->buttons[Piece_PEG_YELLOW]  = button_make( L"Yell.", SCREENPOS( ul.x, ul.y + 11 ), size, KeyInput_3, true );
    comp->buttons[Piece_PEG_CYAN]    = button_make( L"Cyan", SCREENPOS( ul.x, ul.y + 14 ), size, KeyInput_4, true );
    comp->buttons[Piece_PEG_MAGENTA] = button_make( L"Purp.", SCREENPOS( ul.x, ul.y + 17 ), size, KeyInput_5, true );
    comp->buttons[Piece_PEG_BLUE]    = button_make( L"Blue", SCREENPOS( ul.x, ul.y + 20 ), size, KeyInput_6, true );
    comp->buttons[Piece_PEG_WHITE]   = button_make( L"White", SCREENPOS( ul.x, ul.y + 23 ), size, KeyInput_7, true );

    comp->dragPos = SCREENPOS( 0, 0 );
    memset( comp->dragSave, 0, sizeof( comp->dragSave ) );

    return (struct ComponentHeader *)comp;
}
