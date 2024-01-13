#include "widgets/widget_board.h"

#include "components/component_header.h"
#include "mastermind.h"
#include "rect.h"
#include "terminal/terminal.h"

#include <stdlib.h>
#include <string.h>


struct WidgetBoard
{
	struct ComponentHeader header;

	u8 lastDisplayedTurn;
	usize totalBoardWidth;
	usize spacesBetweenBoard;
	usize totalPegSize;
	usize totalPinSize;
	struct Rect box;
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


static void draw_peg( screenpos const ul, enum PegId const id, bool const hidden )
{
	bool const isEmpty = ( id == PegId_Empty );

	if ( hidden )
	{
		style_update( STYLE( FGColor_BRIGHT_BLACK ) );

		cursor_update_yx( ul.y, ul.x );
		term_write( L"\x1B[2m,d||b." );
		cursor_update_yx( ul.y + 1, ul.x );
		term_write( L"O ?? O" );
		cursor_update_yx( ul.y + 2, ul.x );
		term_write( L"`Y||P'\x1B[0m" );
	}
	else
	{
		style_update( STYLE( peg_get_color( id, false /* TODO be relevant with the board */ ) ) );
		cursor_update_yx( ul.y, ul.x );
		term_write( isEmpty ? L",:'':." : L",d||b." );
		cursor_update_yx( ul.y + 1, ul.x );
		term_write( isEmpty ? L":    :" : L"OOOOOO" );
		cursor_update_yx( ul.y + 2, ul.x );
		term_write( isEmpty ? L"`:,,:'" : L"`Y||P'" );
	}
}

static void draw_pin( screenpos const ul, enum PinId const id )
{
	bool const isEmpty = ( id == PinId_INCORRECT );

	style_update( pin_get_style( id ) );

	cursor_update_yx( ul.y, ul.x );
	term_write( isEmpty ? L".''." : L",db." );
	cursor_update_yx( ul.y + 1, ul.x );
	term_write( isEmpty ? L"`,,'" : L"`YP'" );
}


static inline void draw_character_n_times( utf16 const character, usize const nTimes )
{
	for ( usize x = 0; x < nTimes; ++x )
	{
		term_write( L"%lc", character );
	}
}

static void draw_row_board( screenpos const ul, u16 const nbPegs, u16 const nbPins )
{
	style_update( STYLE( FGColor_BRIGHT_BLUE ) );

	u16 const nbPegSpaces = nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	u16 const nbPinSpaces = ( ( nbPins + 1 ) / 2 ) * ( PIN_WIDTH + PIN_INTERSPACE ) - PIN_INTERSPACE;

	cursor_update_yx( ul.y, ul.x );
	term_write( L"################" );
	for ( u16 x = 0; x < nbPegSpaces + nbPinSpaces; ++x ) term_write( L"#" );

	cursor_update_yx( ul.y + 1, ul.x );
	term_write( L"### %*lc ####  %*lc  ###", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	cursor_update_yx( ul.y + 2, ul.x );
	term_write( L"##  %*lc  ##   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	cursor_update_yx( ul.y + 3, ul.x );
	term_write( L"##  %*lc  ##   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	cursor_update_yx( ul.y + 4, ul.x );
	term_write( L"##  %*lc  ##   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	cursor_update_yx( ul.y + 5, ul.x );
	term_write( L"### %*lc ####  %*lc  ###", nbPegSpaces, L' ', nbPinSpaces, L' ' );
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

	cursor_update_yx( ul.y + 3, ul.x + 9 + ( nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) ) - PEG_INTERSPACE );
	term_write( L"%02u", turn );
}


static void draw_row_pegs( screenpos const ul, struct Peg const *pegs, u32 const nbPegs, bool const currentTurnDisplayed )
{
	for ( u32 pegIdx = 0; pegIdx < nbPegs; ++pegIdx )
	{
		screenpos pegUL = (screenpos) {
			.x = ul.x + ( pegIdx * ( PEG_WIDTH + PEG_INTERSPACE ) ),
			.y = ul.y
		};
		draw_peg( pegUL, pegs[pegIdx].id, pegs[pegIdx].hidden );

		pegUL.y += PEG_HEIGHT;
		cursor_update_pos( pegUL );
		if ( currentTurnDisplayed && mastermind_get_selection_bar_index( mastermind_get_instance() ) == pegIdx )
		{
			style_update( STYLE( FGColor_BRIGHT_BLUE ) );
			draw_character_n_times( L'-', PEG_WIDTH );
		}
		else
		{
			draw_character_n_times( L' ', PEG_WIDTH );
		}
	}
}


static void draw_row_pins( screenpos const ul, u32 const nbPegs, struct Pin const *pins, u32 const nbPins )
{
	bool const oddNbPins = nbPins % 2 != 0; // Need to add a last empty pin manually

	int const ulX = ul.x + 11 + nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	int ulY = ul.y + 1;

	// first row
	u8 const firstRowLimit = ( nbPins + 1 ) / 2;
	for ( int idx = 0; idx < firstRowLimit; ++idx )
	{
		draw_pin( SCREENPOS( ulX + 5 * idx, ulY ), pins[idx].id );
	}

	ulY += 3;

	// second row, except the last for odds
	for ( int idx = firstRowLimit; idx < nbPins; ++idx )
	{
		draw_pin( SCREENPOS( ulX + 5 * ( idx - firstRowLimit ), ulY ), pins[idx].id );
	}

	if ( oddNbPins )
	{
		draw_pin( SCREENPOS( ulX + 5 * ( nbPins - firstRowLimit ), ulY ), PinId_INCORRECT );
	}
}


static void draw_row( screenpos const ul, u8 turnToDisplay )
{
	u8 const nbPegs = mastermind_get_nb_pieces_per_turn();
	struct Peg const *pegs = mastermind_get_pegs_at_turn( turnToDisplay );
	struct Pin const *pins = mastermind_get_pins_at_turn( turnToDisplay );
	u8 const playerTurn = mastermind_get_player_turn();
	bool const isCurrentTurnDisplayed = ( playerTurn == turnToDisplay );

	draw_row_board( ul, nbPegs, nbPegs );
	draw_row_pegs( SCREENPOS( ul.x + 4, ul.y + 2 ), pegs, nbPegs, isCurrentTurnDisplayed );
	if ( playerTurn > turnToDisplay )
	{
		draw_row_pins( ul, nbPegs, pins, nbPegs );
	}
	draw_row_turn( ul, nbPegs, turnToDisplay, playerTurn );
}


static void draw_header_title( screenpos const ul, struct WidgetBoard const *board )
{
	usize const titleSize = 39;
	usize const spacesBetween = ( board->totalBoardWidth - titleSize ) / 2;
	screenpos const ulTitle = SCREENPOS( ul.x + spacesBetween, ul.y + 1); 

	style_update( STYLE( FGColor_YELLOW ) );

	cursor_update_yx( ulTitle.y, ulTitle.x );
	term_write( L"_  _ ___ ___ ___ ___ ___ _  _ _ _  _ __ " );
	cursor_update_yx( ulTitle.y + 1, ulTitle.x );
	term_write( L"|\\/| |_| [_   |  |_  |_/ |\\/| | |\\ | | \\" );
	cursor_update_yx( ulTitle.y + 2, ulTitle.x );
	term_write( L"|  | | | __]  |  |__ | \\ |  | | | \\| |_/" );
}


static void draw_header_board( screenpos const ul, struct WidgetBoard const *board )
{
	usize const width = board->totalBoardWidth;

	style_update( STYLE( FGColor_BRIGHT_BLUE ) );

	// First line
	cursor_update_yx( ul.y, ul.x );
	term_write( L"   " );
	draw_character_n_times( L'#', width - 6 );
	term_write( L"   " );

	// Second line
	cursor_update_yx( ul.y + 1, ul.x );
	term_write( L" ####" );
	draw_character_n_times( L' ', width - 10 );
	term_write( L"#### " );

	// Third line
	cursor_update_yx( ul.y + 2, ul.x );
	term_write( L" ##" );
	draw_character_n_times( L' ', width - 6 );
	term_write( L"## " );

	// Fourth line
	cursor_update_yx( ul.y + 3, ul.x );
	term_write( L"###" );
	draw_character_n_times( L' ', width - 6 );
	term_write( L"###" );

	// Fifth line
	cursor_update_yx( ul.y + 4, ul.x );
	term_write( L"#####" );
	draw_character_n_times( L' ', width - 10 );
	term_write( L"#####" );

	// Sixth line
	cursor_update_yx( ul.y + 5, ul.x );
	draw_character_n_times( L'#', width );

	draw_header_title( ul, board );
}


static void draw_solution_pegs( screenpos const ul, struct WidgetBoard const *board )
{
	usize const pegsSize = board->totalPegSize;
	usize const spacesBetween = ( board->totalBoardWidth - pegsSize ) / 2; // ul.x is not the beginning of the board though, hence the decalage.
	screenpos const ulSolution = SCREENPOS( ul.x + spacesBetween, ul.y + 1 );

	struct Peg const *solution = mastermind_get_solution();
	draw_row_pegs( ulSolution, solution, mastermind_get_nb_pieces_per_turn(), false );
}


static void draw_footer_board( screenpos const ul, struct WidgetBoard const *board )
{
	usize const width = board->totalBoardWidth;

	style_update( STYLE( FGColor_BRIGHT_BLUE ) );

	// First line
	cursor_update_yx( ul.y, ul.x );
	term_write( L"######" );
	draw_character_n_times( L' ', width - 12 );
	term_write( L"######" );

	// Second line
	cursor_update_yx( ul.y + 1, ul.x );
	term_write( L"####" );
	draw_character_n_times( L' ', width - 8 );
	term_write( L"####" );

	// Thirth line
	cursor_update_yx( ul.y + 2, ul.x );
	term_write( L"###" );
	draw_character_n_times( L' ', width - 6 );
	term_write( L"###" );

	// Forth line
	cursor_update_yx( ul.y + 3, ul.x );
	term_write( L" ###" );
	draw_character_n_times( L' ', width - 8 );
	term_write( L"### " );

	// Fifth line
	cursor_update_yx( ul.y + 4, ul.x );
	term_write( L"  ####" );
	draw_character_n_times( L' ', width - 12 );
	term_write( L"####  " );

	// Sixth line
	cursor_update_yx( ul.y + 5, ul.x );
	term_write( L"    " );
	draw_character_n_times( L'#', width - 8 );
	term_write( L"    " );

	draw_solution_pegs( ul, board );
}


static void calculate_board_display( struct ComponentHeader *widget )
{
	struct WidgetBoard *board = (struct WidgetBoard *)widget;

	usize const nbPegs = mastermind_get_nb_pieces_per_turn();

	usize const totalWidth = board->box.size.w;
	board->totalPegSize = nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	board->totalPinSize = ( ( nbPegs + 1 ) / 2 ) * ( PIN_WIDTH + PIN_INTERSPACE ) - PIN_INTERSPACE;
	usize const baseBoardSize =  4 /*leftBoardPart*/ + 7 /*middle part*/ + 5 /*right side*/;

	board->totalBoardWidth = baseBoardSize + board->totalPegSize + board->totalPinSize;
	board->spacesBetweenBoard = ( totalWidth - board->totalBoardWidth ) / 2;
}


static void on_refresh_callback( struct ComponentHeader const *widget )
{
	struct WidgetBoard const *board = (struct WidgetBoard const *)widget;
	screenpos const ul = rect_get_ul_corner( &board->box );

	int x = ul.x + board->spacesBetweenBoard;
	int y = ul.y;

	if ( board->lastDisplayedTurn == 3 )
	{
		draw_header_board( SCREENPOS( x, y ), board );
	}
	else
	{
		draw_row( SCREENPOS( x, y ), board->lastDisplayedTurn - 3 );
	}

	draw_row( SCREENPOS( x, y + ROW_HEIGHT ), board->lastDisplayedTurn - 2 );
	draw_row( SCREENPOS( x, y + ROW_HEIGHT * 2 ), board->lastDisplayedTurn - 1 );

	if ( board->lastDisplayedTurn == mastermind_get_total_turns() + 1 )
	{
		style_update( STYLE( FGColor_BRIGHT_BLUE ) );
		cursor_update_yx( y + ROW_HEIGHT * 3, x );
		draw_character_n_times( L'#', board->totalBoardWidth );
		draw_footer_board( SCREENPOS( x, y + ROW_HEIGHT * 3 + 1 ), board );
	}
	else
	{
		draw_row( SCREENPOS( x, y + ROW_HEIGHT * 3 ), board->lastDisplayedTurn );
		cursor_update_yx( y + ROW_HEIGHT * 4, x );
		style_update( STYLE( FGColor_BRIGHT_BLUE ) );
		draw_character_n_times( L'#', board->totalBoardWidth );
	}
}


static void on_game_update_callback( struct ComponentHeader *widget, enum GameUpdateType type )
{
	if ( type == GameUpdateType_GAME_NEW )
	{		
		calculate_board_display( widget );
		((struct WidgetBoard *)widget)->lastDisplayedTurn = 3;
		widget->enabled = true;
		widget->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_GAME_FINISHED )
	{
		((struct WidgetBoard *)widget)->lastDisplayedTurn = mastermind_get_total_turns() + 1;
		widget->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_SELECTION_BAR_MOVED )
	{
		// TODO: Only remove the old selection bar, and display the new one instead of redrawing the whole board.
		widget->refreshNeeded = true;
	}
}


static bool on_input_received_callback( struct ComponentHeader *widget, enum KeyInput input )
{
	struct WidgetBoard *board = (struct WidgetBoard *)widget;

	// TODO:
	// How do we disable the history buttons of the button widget if we can't go up anymore ? Same question for the down
	// Perhaps we could enlarge the GameUpdateType Scope to give the possibility to a widget to send an event as well ?
	// Because here we don't have any relation with the game, it's a widget that needs to change its behaviour because of
	// another widget, a.k.a disable the history button depending on where we are.

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
			if ( board->lastDisplayedTurn > 3 )
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
    struct WidgetBoard *const board = malloc( sizeof( struct WidgetBoard ) );
    if ( !board ) return NULL;
	memset( board, 0, sizeof( *board ) );

	struct ComponentHeader *const widget = &board->header;

    widget->id = ComponentId_BOARD;
	widget->enabled = false;
	widget->refreshNeeded = false;

	board->box = rect_make( SCREENPOS( 2, 3 ), VEC2U16( TOTAL_BOARD_SIZE, 25 ) );

	widget->callbacks.refreshCb = on_refresh_callback;
	widget->callbacks.gameUpdateCb = on_game_update_callback;
	widget->callbacks.inputReceivedCb = on_input_received_callback;

	board->lastDisplayedTurn = 3; // Title + 1st turn + 2nd turn

	return (struct ComponentHeader *)board;
}
