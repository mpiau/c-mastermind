#include "components/component_board.h"

#include "components/component_header.h"
#include "mastermind.h"
#include "rect.h"
#include "terminal/terminal.h"
#include "keybindings.h"
#include "ui.h"

#include <stdlib.h>
#include <string.h>

enum // Constants
{
	ROWS_DISPLAYED = 4
};

enum ButtonIdx
{
	ButtonIdx_CONFIRM_TURN,
	ButtonIdx_RESET_TURN,
	ButtonIdx_ABANDON_GAME,

	ButtonIdx_Count
};

struct BoardRow
{
	u8 displayedTurn;
	struct Rect pegs[Mastermind_MAX_PIECES_PER_TURN];
	screenpos pins[Mastermind_MAX_PIECES_PER_TURN];
	screenpos turnPos;
	u8 activeHoveredPegIdx;
};

static const u8 S_INVALID_IDX = (u8)-1;


struct ComponentBoard
{
	struct ComponentHeader header;

	struct Rect box;
	struct BoardRow rows[ROWS_DISPLAYED];
	u8 nbPiecesPerTurn;
	u64 buttons[ButtonIdx_Count];
};

//	struct Rect pegs[ROWS_DISPLAYED][Mastermind_MAX_PIECES_PER_TURN];
//	struct Rect turns[ROWS_DISPLAYED];
//	struct Rect pins[ROWS_DISPLAYED][Mastermind_MAX_PIECES_PER_TURN];
//	u8 firstTurnOnScreen;
//	u8 lastTurnOnScreen;

#define CAST_TO_COMP( _header ) ( ( struct ComponentBoard * )( _header ) )

enum // Constants
{
	PEG_WIDTH = 6,
	PEG_HEIGHT = 3,
	PEG_INTERSPACE = 2,
	PIN_WIDTH = 4,
	PIN_HEIGHT = 2,
	PIN_INTERSPACE = 1,
	ROW_HEIGHT = 6,

	TOTAL_BOARD_WIDTH = 78,
};


static inline void draw_character_n_times( utf16 const character, usize const n )
{
	for ( usize x = 0; x < n; ++x )
	{
		term_write( L"%lc", character );
	}
}


/*
static inline bool is_player_turn_displayed( struct ComponentBoard *const comp, usize const playerTurn )
{
	return ( comp->firstTurnOnScreen <= playerTurn && comp->lastTurnOnScreen >= playerTurn );
}





static void calculate_rect_positions( struct ComponentBoard *const comp )
{
	screenpos const ul = rect_get_ul_corner( &comp->box );
	for ( usize y = 0; y < ROWS_DISPLAYED; ++y )
	{
		for ( usize x = 0; x < comp->nbPiecesPerTurn; ++x )
		{
			struct Rect *rect = &comp->pegs[y][x];
			screenpos const pos = {
				.x = ul.x + 8 * x + ( ( Mastermind_MAX_PIECES_PER_TURN - comp->nbPiecesPerTurn ) * 2 ),
				.y = ul.y + 1 + 6 * y
			};
			*rect = rect_make( pos, VEC2U16( 6, 3 ) );
		}

		// Turns
		{
			screenpos const lastPegBR = rect_get_br_corner( &comp->pegs[y][comp->nbPiecesPerTurn - 1] );
			screenpos turnUL = SCREENPOS( lastPegBR.x + 2, lastPegBR.y - 1 );
			comp->turns[y] = rect_make( turnUL, VEC2U16( 7, 1 ) );
		}

		// Pins
		{
			screenpos const turnBR = rect_get_br_corner( &comp->turns[y] );
			screenpos const pinUL = SCREENPOS( turnBR.x + 2, turnBR.y - 1 );

			// first row
			u8 const firstRowLimit = ( comp->nbPiecesPerTurn + 1 ) / 2;
			for ( int x = 0; x < firstRowLimit; ++x )
			{
				comp->pins[y][x] = rect_make( SCREENPOS( pinUL.x + 5 * x, ul.y ), VEC2U16( 4, 2 ) );
			}
			for ( int x = firstRowLimit; x < comp->nbPiecesPerTurn; ++x )
			{
				comp->pins[y][x] = rect_make( SCREENPOS( pinUL.x + 5 * x, ul.y + 3 ), VEC2U16( 4, 2 ) );
			}
		}
	}
}*/


/*static void sync_board_with_game_status( struct ComponentBoard *const comp )
{
	for ( usize y = 0; y < ARR_COUNT( comp->pieces ); ++y )
	{
		for ( usize x = 0; x < comp->nbPiecesPerTurn; ++x )
		{
			comp->pieces[y][x] = mastermind_get_peg( comp->firstTurnOnScreen + y, x );
		}
	}
}*/


/*static void draw_row_turn( screenpos const ul, u16 const nbPegs, u32 const turn, u32 const playerTurn )
{
	if ( turn == playerTurn )
	{
		style_update( STYLE( FGColor_YELLOW ) );
	}
	else if ( turn < playerTurn )
	{
		style_update( STYLE( FGColor_WHITE ) );
	}
	else
	{
		style_update( STYLE( FGColor_BRIGHT_BLACK ) );
	}

	cursor_update_yx( ul.y + 2, ul.x + 2 + ( nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) ) - PEG_INTERSPACE );
	term_write( L"Turn %02u", turn );
}


static void draw_row_pegs( screenpos const ul, gamepiece const *pegs, u32 const nbPegs, bool const currentTurnDisplayed )
{
	for ( u32 pegIdx = 0; pegIdx < nbPegs; ++pegIdx )
	{
		screenpos pegUL = (screenpos) {
			.x = ul.x + ( pegIdx * ( PEG_WIDTH + PEG_INTERSPACE ) ),
			.y = ul.y
		};
		piece_write_6x3( pegUL, pegs[pegIdx] );

		pegUL.y += PEG_HEIGHT;
		cursor_update_pos( pegUL );
		if ( currentTurnDisplayed && mastermind_get_selection_bar_index( mastermind_get_instance() ) == pegIdx )
		{
			style_update( STYLE( FGColor_BRIGHT_BLACK ) );
			term_write( L"⊲────⊳" );
		}
		else
		{
			term_write( L"      " );
		}
	}
}


static void draw_row_pins( screenpos const ul, u32 const nbPegs, gamepiece const *pins, u32 const nbPins )
{
	bool const oddNbPins = nbPins % 2 != 0; // Need to add a last empty pin manually

	int const ulX = ul.x + 11 + nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	int ulY = ul.y;// + 1;

	// first row
	u8 const firstRowLimit = ( nbPins + 1 ) / 2;
	for ( int idx = 0; idx < firstRowLimit; ++idx )
	{
		piece_write_4x2( SCREENPOS( ulX + 5 * idx, ulY ), pins[idx] );
	}

	ulY += 3;

	// second row, except the last for odds
	for ( int idx = firstRowLimit; idx < nbPins; ++idx )
	{
		piece_write_4x2( SCREENPOS( ulX + 5 * ( idx - firstRowLimit ), ulY ), pins[idx] );
	}

	if ( oddNbPins )
	{
		piece_write_4x2( SCREENPOS( ulX + 5 * ( nbPins - firstRowLimit ), ulY ), Piece_PIN_INCORRECT );
	}
}



static void draw_solution_pegs( screenpos const ul, struct ComponentBoard const *comp )
{*/
/*	usize const pegsSize = comp->totalPegSize;
	usize const spacesBetween = ( comp->totalBoardWidth - pegsSize ) / 2; // ul.x is not the beginning of the comp though, hence the decalage.
	screenpos const ulSolution = SCREENPOS( ul.x + spacesBetween, ul.y + 1 );

	gamepiece const *solution = mastermind_get_solution();
	draw_row_pegs( ulSolution, solution, mastermind_get_nb_pieces_per_turn(), false );*/
/*}*/


/*static void calculate_board_display( struct ComponentHeader *header )
{
	struct ComponentBoard *comp = (struct ComponentBoard *)header;

	usize const nbPegs = mastermind_get_nb_pieces_per_turn();

	usize const totalWidth = comp->box.size.w;
	comp->totalPegSize = nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	comp->totalPinSize = ( ( nbPegs + 1 ) / 2 ) * ( PIN_WIDTH + PIN_INTERSPACE ) - PIN_INTERSPACE;
	usize const baseBoardSize =  4 + 7 + 5;

	comp->totalBoardWidth = baseBoardSize + comp->totalPegSize + comp->totalPinSize;
	comp->spacesBetweenBoard = ( totalWidth - comp->totalBoardWidth ) / 2;
}*/


/*static void row_v2( screenpos ul, int turnToDisplay )
{
	u8 const nbPegs = mastermind_get_nb_pieces_per_turn();
	gamepiece const *pegs = mastermind_get_pegs_at_turn( turnToDisplay );
	gamepiece const *pins = mastermind_get_pins_at_turn( turnToDisplay );
	u8 const playerTurn = mastermind_get_player_turn();
	bool const isCurrentTurnDisplayed = ( playerTurn == turnToDisplay );

	draw_row_pegs( SCREENPOS( ul.x, ul.y + 1 ), pegs, nbPegs, isCurrentTurnDisplayed );
	if ( playerTurn > turnToDisplay )
	{
		draw_row_pins( ul, nbPegs, pins, nbPegs );
	}
	draw_row_turn( ul, nbPegs, turnToDisplay, playerTurn );
}

static void draw_board_peg( struct ComponentBoard const *comp, usize turn, usize index )
{
}


static void draw_board_pegs_row( struct ComponentBoard const *comp, usize playerTurn )
{
	int yIndex = playerTurn - comp->firstTurnOnScreen;
	if ( yIndex < 0 || yIndex >= ARR_COUNT( comp->pegs ) ) return; // Row not in screen

	for ( usize x = 0; x < comp->nbPiecesPerTurn; ++x )
	{
//		draw_board_peg( )
	}
}


static void draw_board_content( struct ComponentBoard const *comp )
{
	for ( usize y = 0; y < ROWS_DISPLAYED; ++y )
	{
		for ( usize x = 0; x < comp->nbPiecesPerTurn; ++x )
		{
			screenpos pos = rect_get_ul_corner( &comp->pegs[y][x] );
			piece_write_6x3( pos, mastermind_get_peg( comp->firstTurnOnScreen + y, x ) );
		}
	}
*/
/*		{
			screenpos const lastPegBR = rect_get_br_corner( &comp->pegs[y][comp->nbPiecesPerTurn - 1] );
			screenpos turnUL = SCREENPOS( lastPegBR.x + 2, lastPegBR.y - 1 );
			comp->turns[y] = rect_make( turnUL, VEC2U16( 7, 1 ) );
		}

		// Pins
		{
			screenpos const turnBR = rect_get_br_corner( &comp->turns[y] );
			screenpos const pinUL = SCREENPOS( turnBR.x + 2, turnBR.y - 1 );

			// first row
			u8 const firstRowLimit = ( comp->nbPiecesPerTurn + 1 ) / 2;
			for ( int x = 0; x < firstRowLimit; ++x )
			{
				comp->pins[y][x] = rect_make( SCREENPOS( pinUL.x + 5 * x, ul.y ), VEC2U16( 4, 2 ) );
			}
			for ( int x = firstRowLimit; x < comp->nbPiecesPerTurn; ++x )
			{
				comp->pins[y][x] = rect_make( SCREENPOS( pinUL.x + 5 * x, ul.y + 3 ), VEC2U16( 4, 2 ) );
			}
		}
	}*/
/*	screenpos ul = rect_get_ul_corner( &comp->box );
	ul.y += 1;
	ul.x += 2;
	row_v2( SCREENPOS( ul.x + 1, ul.y ), comp->firstTurnOnScreen );
	ul.y += 6;
	row_v2( SCREENPOS( ul.x + 1, ul.y ), comp->firstTurnOnScreen + 1 );
	ul.y += 6;
	row_v2( SCREENPOS( ul.x + 1, ul.y ), comp->firstTurnOnScreen + 2 );
	ul.y += 6;
	if ( comp->firstTurnOnScreen <= mastermind_get_total_turns() - 3 )
	{
		row_v2( SCREENPOS( ul.x + 1, ul.y ), comp->firstTurnOnScreen + 3 );
	}
	else
	{
		draw_solution_pegs( SCREENPOS( ul.x + 1, ul.y ), comp );
	}*/
/*}


static void clear_board_content( struct ComponentBoard const *comp )
{

}


static void on_game_update_callback( struct ComponentHeader *header, enum GameUpdateType type )
{
	struct ComponentBoard *comp = (struct ComponentBoard *)header;

	if ( type == GameUpdateType_GAME_NEW )
	{
		comp->firstTurnOnScreen = 1;

		// check if number of pegs is differents.
		// if yes, we need to clear everything and calculate position again.
		if ( comp->nbPiecesPerTurn != mastermind_get_nb_pieces_per_turn() )
		{
			comp->nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();
			calculate_rect_positions( comp );
		}
//		sync_board_with_game_status( comp );


//		calculate_board_display( header );
		for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
		{
			uibutton_show( comp->buttons[idx] );
		}
		header->enabled = true;
//		header->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_GAME_FINISHED )
	{
		comp->firstTurnOnScreen = mastermind_get_total_turns() - 3;
		for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
		{
			uibutton_hide( comp->buttons[idx] );
		}
//		header->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_SELECTION_BAR_MOVED || type == GameUpdateType_PEG_ADDED || type == GameUpdateType_NEXT_TURN )
	{
//		header->refreshNeeded = true;
	}
}


static bool on_input_received_callback( struct ComponentHeader *header, enum KeyInput input )
{
	struct ComponentBoard *comp = (struct ComponentBoard *)header;

	switch( input )
	{
		case KeyInput_ARROW_DOWN:
		{
			usize const nbTurns = mastermind_get_total_turns();
			if ( comp->firstTurnOnScreen < nbTurns - 3 )
			{
				comp->firstTurnOnScreen += 1;
				header->refreshNeeded = true;
			}
			return true;
		}		
		case KeyInput_ARROW_UP:
		{
			if ( comp->firstTurnOnScreen > 1 )
			{
				comp->firstTurnOnScreen -= 1;
				header->refreshNeeded = true;
			}
			return true;
		}
		case KeyInput_MOUSE_BTN_LEFT:
		{
			for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
			{
				if ( uibutton_check_pressed( comp->buttons[idx] ) )
				{
					return true;
				}
			}
		}
	}
	return false;
}

*/


static void draw_internal_board_lines( struct ComponentBoard *comp )
{
	screenpos const ul = rect_get_ul_corner( &comp->box );

	// Lines separating the turns
	style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
	for( usize idx = 0; idx < 3; idx++ )
	{
		cursor_update_yx( ul.y + 6 * ( idx + 1 ), ul.x + 2 ); 
		draw_character_n_times( L'─', TOTAL_BOARD_WIDTH - 4 );
	}

	// Line that separates buttons from the rest of the game
	cursor_update_yx( ul.y + comp->box.size.h - 3, ul.x );
	style_update( STYLE( FGColor_BRIGHT_BLACK ) );
	term_write( L"%lc", L'├' );
	draw_character_n_times( L'─', TOTAL_BOARD_WIDTH - 2 );
	term_write( L"%lc", L'┤' );
}


static void enable_callback( struct ComponentHeader *header )
{
	struct ComponentBoard *comp = (struct ComponentBoard *)header;
	rect_draw_borders( &comp->box, L"Mastermind Board" );
	draw_internal_board_lines( comp );
	// TODO: draw_board_content( comp );

	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_show( comp->buttons[idx] );
	}
}


static void disable_callback( struct ComponentHeader *header )
{
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_hide( CAST_TO_COMP( header )->buttons[idx] );
	}
	rect_clear( &CAST_TO_COMP( header )->box );
}


static void check_peg_hovered( struct ComponentBoard *comp, struct BoardRow *row, screenpos pos )
{
	for ( usize idx = 0; idx < comp->nbPiecesPerTurn; ++idx )
	{
		struct Rect *rect = &row->pegs[idx];
		if ( rect_is_inside( rect, pos ) )
		{
			if ( row->activeHoveredPegIdx == idx ) return;
			if ( row->activeHoveredPegIdx != S_INVALID_IDX )
			{
				screenpos const ulOldHovered = rect_get_ul_corner( &row->pegs[row->activeHoveredPegIdx] );
				piece_write_6x3( ulOldHovered, mastermind_get_peg( row->displayedTurn, row->activeHoveredPegIdx ), false );
			}
			row->activeHoveredPegIdx = idx;
			piece_write_6x3( rect_get_ul_corner( rect ), mastermind_get_peg( row->displayedTurn, row->activeHoveredPegIdx ), true );
			return;
		}
	}

	// In case no peg is being hovered, but one is marked as hovered.
	if ( row->activeHoveredPegIdx != S_INVALID_IDX )
	{
		struct Rect *rect = &row->pegs[row->activeHoveredPegIdx];
		screenpos ul = rect_get_ul_corner( rect );

		piece_write_6x3( ul, mastermind_get_peg( row->displayedTurn, row->activeHoveredPegIdx ), false );
		row->activeHoveredPegIdx = S_INVALID_IDX;
	}
}


static void on_mouse_move_callback( struct ComponentHeader *header, screenpos const pos )
{
	struct ComponentBoard *comp = CAST_TO_COMP( header );
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		if ( uibutton_check_hovered( comp->buttons[idx], pos ) )
		{
			break;
		}
	}

	for ( usize idx = 0; idx < ARR_COUNT( comp->rows ); ++idx )
	{
		struct BoardRow *row = &comp->rows[idx];
		if ( row->displayedTurn == mastermind_get_player_turn() )
		{
			check_peg_hovered( comp, row, pos );
			break;
		}
	}
}


struct ComponentHeader *component_board_create( void )
{
	struct ComponentBoard *const comp = calloc( 1, sizeof( struct ComponentBoard ) );
    if ( !comp ) return NULL;

	comp->header.id = ComponentId_BOARD;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
	callbacks->enableCb = enable_callback;
	callbacks->disableCb = disable_callback;
    // callbacks->eventReceivedCb = on_game_update_callback;
    // callbacks->inputReceivedCb = on_input_received_callback;
	callbacks->mouseMoveCb = on_mouse_move_callback;


	comp->box = rect_make( SCREENPOS( 17, 2 ), VEC2U16( TOTAL_BOARD_WIDTH, 27 ) );
	comp->nbPiecesPerTurn = 0;

	screenpos const bul = SCREENPOS( 18, 27 );
    comp->buttons[ButtonIdx_CONFIRM_TURN] = uibutton_register( L"Confirm Turn", SCREENPOS( bul.x + 3, bul.y ), VEC2U16( 19, 1 ), Keybinding_CONFIRM_TURN, NULL, true );
    comp->buttons[ButtonIdx_RESET_TURN]   = uibutton_register( L"Reset Turn", SCREENPOS( bul.x + 25, bul.y ), VEC2U16( 13, 1 ), Keybinding_RESET_TURN, NULL, true );
    comp->buttons[ButtonIdx_ABANDON_GAME] = uibutton_register( L"Abandon Game", SCREENPOS( bul.x + 58, bul.y ), VEC2U16( 15, 1 ), Keybinding_ABANDON_GAME, NULL, true );

	return (struct ComponentHeader *)comp;
}
