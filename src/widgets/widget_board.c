#include "widgets/widget_board.h"

#include "components/component_header.h"
#include "mastermind.h"
#include "rect.h"
#include "terminal/terminal.h"

#include <stdlib.h>
#include <string.h>


struct ComponentBoard
{
	struct ComponentHeader header;

	u8 lastDisplayedTurn;
	usize totalBoardWidth;
	usize spacesBetweenBoard;
	usize totalPegSize;
	usize totalPinSize;
	struct Rect box;
	struct Rect currentTurnPegs[Mastermind_MAX_PIECES_PER_TURN];
};


enum // Constants
{
	PEG_WIDTH = 6,
	PEG_HEIGHT = 3,
	PEG_INTERSPACE = 2,
	PIN_WIDTH = 4,
	PIN_HEIGHT = 2,
	PIN_INTERSPACE = 1,
	ROW_HEIGHT = 6,

	TOTAL_BOARD_SIZE = 78,
};


static inline void draw_character_n_times( utf16 const character, usize const nTimes )
{
	for ( usize x = 0; x < nTimes; ++x )
	{
		term_write( L"%lc", character );
	}
}


static void draw_row_turn( screenpos const ul, u16 const nbPegs, u32 const turn, u32 const playerTurn )
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



static void draw_solution_pegs( screenpos const ul, struct ComponentBoard const *board )
{
	usize const pegsSize = board->totalPegSize;
	usize const spacesBetween = ( board->totalBoardWidth - pegsSize ) / 2; // ul.x is not the beginning of the board though, hence the decalage.
	screenpos const ulSolution = SCREENPOS( ul.x + spacesBetween, ul.y + 1 );

	gamepiece const *solution = mastermind_get_solution();
	draw_row_pegs( ulSolution, solution, mastermind_get_nb_pieces_per_turn(), false );
}


static void calculate_board_display( struct ComponentHeader *widget )
{
	struct ComponentBoard *board = (struct ComponentBoard *)widget;

	usize const nbPegs = mastermind_get_nb_pieces_per_turn();

	usize const totalWidth = board->box.size.w;
	board->totalPegSize = nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	board->totalPinSize = ( ( nbPegs + 1 ) / 2 ) * ( PIN_WIDTH + PIN_INTERSPACE ) - PIN_INTERSPACE;
	usize const baseBoardSize =  4 /*leftBoardPart*/ + 7 /*middle part*/ + 5 /*right side*/;

	board->totalBoardWidth = baseBoardSize + board->totalPegSize + board->totalPinSize;
	board->spacesBetweenBoard = ( totalWidth - board->totalBoardWidth ) / 2;
}


static void row_v2( screenpos ul, int turnToDisplay )
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


static void on_refresh_callback( struct ComponentHeader const *widget )
{
	struct ComponentBoard const *board = (struct ComponentBoard const *)widget;
	screenpos ul = rect_get_ul_corner( &board->box );
	
	rect_draw_borders( &board->box, L"Mastermind Board" );

	ul.y += 1;
	ul.x += 2;
	row_v2( SCREENPOS( ul.x + 1, ul.y ), board->lastDisplayedTurn - 3 );

	ul.y += 5;
	cursor_update_pos( ul ); style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
	draw_character_n_times( L'─', 74 );
	ul.y += 1;

	row_v2( SCREENPOS( ul.x + 1, ul.y ), board->lastDisplayedTurn - 2 );
	ul.y += 5;
	cursor_update_pos( ul ); style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
	draw_character_n_times( L'─', 74 );
	ul.y += 1;
	row_v2( SCREENPOS( ul.x + 1, ul.y ), board->lastDisplayedTurn - 1 );
	ul.y += 5;
	cursor_update_pos( ul ); style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
	draw_character_n_times( L'─', 74 );
	ul.y += 1;
	if ( board->lastDisplayedTurn <= mastermind_get_total_turns() )
	{
		row_v2( SCREENPOS( ul.x + 1, ul.y ), board->lastDisplayedTurn );
	}
	ul.y += 5;

	ul.x -= 2;
	cursor_update_pos( ul );
	style_update( STYLE( FGColor_BRIGHT_BLACK ) );
	term_write( L"%lc", L'├' );
	draw_character_n_times( L'─', 76 );
	term_write( L"%lc", L'┤' );
}


static void on_game_update_callback( struct ComponentHeader *widget, enum GameUpdateType type )
{
	if ( type == GameUpdateType_GAME_NEW )
	{		
		calculate_board_display( widget );
		((struct ComponentBoard *)widget)->lastDisplayedTurn = 4;
		widget->enabled = true;
		widget->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_GAME_FINISHED )
	{
		((struct ComponentBoard *)widget)->lastDisplayedTurn = mastermind_get_total_turns() + 1;
		widget->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_SELECTION_BAR_MOVED || type == GameUpdateType_PEG_ADDED || type == GameUpdateType_NEXT_TURN )
	{
		widget->refreshNeeded = true;
	}
}


static bool on_input_received_callback( struct ComponentHeader *widget, enum KeyInput input )
{
	struct ComponentBoard *board = (struct ComponentBoard *)widget;

	switch( input )
	{
		case KeyInput_ARROW_DOWN:
		{
			usize const nbTurns = mastermind_get_total_turns();
			if ( board->lastDisplayedTurn < nbTurns + 1 )
			{
				board->lastDisplayedTurn += 1;
				widget->refreshNeeded = true;
			}
			return true;
		}		
		case KeyInput_ARROW_UP:
		{
			if ( board->lastDisplayedTurn > 4 )
			{
				board->lastDisplayedTurn -= 1;
				widget->refreshNeeded = true;
			}
			return true;
		}	
	}
	return false;
}


struct ComponentHeader *widget_board_create( void )
{
	struct ComponentBoard *const comp = calloc( 1, sizeof( struct ComponentBoard ) );
    if ( !comp ) return NULL;

    component_make_header( &comp->header, ComponentId_BOARD, false );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->refreshCb = on_refresh_callback;
    callbacks->gameUpdateCb = on_game_update_callback;
    callbacks->inputReceivedCb = on_input_received_callback;


	comp->box = rect_make( SCREENPOS( 17, 2 ), VEC2U16( TOTAL_BOARD_SIZE, 27 ) );
	// Set all the Rect for the pegs emplacements
	// 
	comp->lastDisplayedTurn = 4;

	return (struct ComponentHeader *)comp;
}
