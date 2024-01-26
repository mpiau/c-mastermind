#include "components/component_peg_selection.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "mastermind.h"
#include "rect.h"
#include "game.h"
#include "mouse.h"
#include "ui.h"
#include "keybindings.h"

#include <stdlib.h>
#include <string.h>

enum ButtonIdx
{
    ButtonIdx_PEG_BLACK,
    ButtonIdx_PEG_RED,
    ButtonIdx_PEG_GREEN,
    ButtonIdx_PEG_YELLOW,
    ButtonIdx_PEG_CYAN,
    ButtonIdx_PEG_MAGENTA,
    ButtonIdx_PEG_BLUE,
    ButtonIdx_PEG_WHITE,

    ButtonIdx_Count
};


struct ComponentPegSelection
{
    struct ComponentHeader header;
    u64 buttons[ButtonIdx_Count];
    struct Rect box;


//    enum Piece hovered;
//    enum Piece selected;

//    struct Button buttonsDeprecated[Piece_PEGCount];

//    struct Character dragSave[2][4];
//    screenpos dragPos;
};
#define CAST_TO_COMP( _header ) ( ( struct ComponentPegSelection * )( _header ) )

/*
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
}*/

/*
static void write_peg_button( struct Button const *button, gamepiece const piece, bool const isHovered )
{
    button_write( button, isHovered );

    screenpos ul = rect_get_ul_corner( &button->rect );
    ul.x += 6;
    piece_write_4x2( ul, piece );
}
*/

static bool input_received_callback( struct ComponentHeader *header, enum KeyInput const input )
{
    struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;

   	if ( input == KeyInput_MOUSE_BTN_LEFT )
    {
        for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
        {
            if ( uibutton_clicked( CAST_TO_COMP( header )->buttons[idx] ) )
            {
                return true;
            }
        }
    }

    // Dragged peg !

    return false;
/*
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
        button_disable( &comp->buttonsDeprecated[idx] );
        button_write( &comp->buttonsDeprecated[idx], false );
        return true;
    }
    else if ( input == KeyInput_MOUSE_BTN_RIGHT && comp->selected != PieceFlag_EMPTY )
    {
        enum Piece idx = ( comp->selected & Piece_MaskColor );
        button_enable( &comp->buttonsDeprecated[idx] );
        button_write( &comp->buttonsDeprecated[idx], false );
        write_saved_content_at_drag_pos( comp );
        comp->selected = PieceFlag_EMPTY;
        write_dragged_peg( comp );
        return true;
    }
    return false;*/
}


static void event_received_callback( struct ComponentHeader *header, enum EventType event, struct EventData const *data )
{
}


static void on_mouse_move_callback( struct ComponentHeader *header, screenpos pos )
{
	struct ComponentPegSelection *comp = CAST_TO_COMP( header );

	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		if ( uibutton_check_hovered( CAST_TO_COMP( header )->buttons[idx], pos ) )
		{
			return;
		}
	}

    // Dragging mechanism

/*
    if ( comp->selected != PieceFlag_EMPTY )
    {
        write_saved_content_at_drag_pos( comp );
        comp->dragPos = mouse_pos();
        comp->dragPos.x -= 1;
        save_content_at_drag_pos( comp );
        write_dragged_peg( comp );
    }
*/
}


static void enable_callback( struct ComponentHeader *header )
{
    struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;
    rect_draw_borders( &comp->box, L"Pegs" );

   	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_show( comp->buttons[idx] );
	    screenpos ul = rect_get_ul_corner( uibutton_get_box( comp->buttons[idx] ) );
	    ul.x += 6;
    	piece_write_4x2( ul, (enum Piece)idx );
	}
}


static void disable_callback( struct ComponentHeader *header )
{
    struct ComponentPegSelection *comp = (struct ComponentPegSelection *)header;

   	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_hide( comp->buttons[idx] );
	}

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
    callbacks->eventReceivedCb = event_received_callback;
    callbacks->mouseMoveCb = on_mouse_move_callback;

    // Specific to the header
//    comp->hovered = PieceFlag_EMPTY;
//    comp->selected = PieceFlag_EMPTY;

    screenpos ul = SCREENPOS( 3, 2 );
    vec2u16 const size = VEC2U16( 10, 2 );
	comp->box = rect_make( ul, VEC2U16( 14, 27 ) );
    ul.x += 2;
    comp->buttons[ButtonIdx_PEG_BLACK]   = uibutton_try_register( L"Grey", SCREENPOS( ul.x, ul.y + 2 ), size, KeyBinding_PEG_BLACK, true );
    comp->buttons[ButtonIdx_PEG_RED]     = uibutton_try_register( L"Red", SCREENPOS( ul.x, ul.y + 5 ), size, KeyBinding_PEG_RED, true );
    comp->buttons[ButtonIdx_PEG_GREEN]   = uibutton_try_register( L"Green", SCREENPOS( ul.x, ul.y + 8 ), size, KeyBinding_PEG_GREEN, true );
    comp->buttons[ButtonIdx_PEG_YELLOW]  = uibutton_try_register( L"Yell.", SCREENPOS( ul.x, ul.y + 11 ), size, KeyBinding_PEG_YELLOW, true );
    comp->buttons[ButtonIdx_PEG_CYAN]    = uibutton_try_register( L"Cyan", SCREENPOS( ul.x, ul.y + 14 ), size, KeyBinding_PEG_CYAN, true );
    comp->buttons[ButtonIdx_PEG_MAGENTA] = uibutton_try_register( L"Purp.", SCREENPOS( ul.x, ul.y + 17 ), size, KeyBinding_PEG_MAGENTA, true );
    comp->buttons[ButtonIdx_PEG_BLUE]    = uibutton_try_register( L"Blue", SCREENPOS( ul.x, ul.y + 20 ), size, KeyBinding_PEG_BLUE, true );
    comp->buttons[ButtonIdx_PEG_WHITE]   = uibutton_try_register( L"White", SCREENPOS( ul.x, ul.y + 23 ), size, KeyBinding_PEG_WHITE, true );

//    comp->dragPos = SCREENPOS( 0, 0 );
//    memset( comp->dragSave, 0, sizeof( comp->dragSave ) );

    return (struct ComponentHeader *)comp;
}
