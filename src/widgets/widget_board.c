#include "widgets/widget_board.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "console.h"

struct WidgetBoard
{
	struct Widget header;

	u8 lastDisplayedTurn;
	usize totalBoardWidth;
	usize spacesBetweenBoard;
	usize totalPegSize;
	usize totalPinSize;
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

	TOTAL_BOARD_SIZE = 84,
};


static void draw_peg( screenpos const ul, enum PegId const id, bool const hidden )
{
	bool const isEmpty = ( id == PegId_Empty );

	if ( hidden )
	{
		console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

		console_cursor_set_position( ul.y, ul.x );
		console_draw( L"\x1B[2m,d||b." );
		console_cursor_set_position( ul.y + 1, ul.x );
		console_draw( L"O ?? O" );
		console_cursor_set_position( ul.y + 2, ul.x );
		console_draw( L"`Y||P'\x1B[0m" );
	}
	else
	{
		console_color_fg( peg_get_color( id, false /* TODO be relevant with the board */ ) );
		console_cursor_set_position( ul.y, ul.x );
		console_draw( isEmpty ? L",:'':." : L",d||b." );
		console_cursor_set_position( ul.y + 1, ul.x );
		console_draw( isEmpty ? L":    :" : L"OOOOOO" );
		console_cursor_set_position( ul.y + 2, ul.x );
		console_draw( isEmpty ? L"`:,,:'" : L"`Y||P'" );
	}
}

static void draw_pin( screenpos const ul, enum PinId const id )
{
	bool const isEmpty = ( id == PinId_INCORRECT );

	console_color_fg( pin_get_color( id ) );

	console_cursor_set_position( ul.y, ul.x );
	console_draw( isEmpty ? L".''." : L",db." );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( isEmpty ? L"`,,'" : L"`YP'" );
}


static inline void draw_character_n_times( utf16 const character, usize const nTimes )
{
	for ( usize x = 0; x < nTimes; ++x )
	{
		console_draw( L"%lc", character );
	}
}

static void draw_row_board( screenpos const ul, u16 const nbPegs, u16 const nbPins )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );

	u16 const nbPegSpaces = nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	u16 const nbPinSpaces = ( ( nbPins + 1 ) / 2 ) * ( PIN_WIDTH + PIN_INTERSPACE ) - PIN_INTERSPACE;

	console_cursor_set_position( ul.y, ul.x );
	console_draw( L"################" );
	for ( u16 x = 0; x < nbPegSpaces + nbPinSpaces; ++x ) console_draw( L"#" );

	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L"### %*lc ####  %*lc  ###", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( L"##  %*lc  ##   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 3, ul.x );
	console_draw( L"##  %*lc  ##   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 4, ul.x );
	console_draw( L"##  %*lc  ##   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 5, ul.x );
	console_draw( L"### %*lc ####  %*lc  ###", nbPegSpaces, L' ', nbPinSpaces, L' ' );
}


static void draw_row_turn( screenpos const ul, u16 const nbPegs, u32 const turn )
{
	console_color_fg( ConsoleColorFG_WHITE );

	console_cursor_set_position( ul.y + 3, ul.x + 9 + ( nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) ) - PEG_INTERSPACE );
	console_draw( L"%02u", turn );
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
		console_setpos( pegUL );
		if ( currentTurnDisplayed && mastermind_get_selection_bar_index( mastermind_get_instance() ) == pegIdx )
		{
			console_color_fg( ConsoleColorFG_CYAN );
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


static void draw_row( screenpos const ul, struct Mastermind const *mastermind, u8 turnToDisplay )
{
	u8 const nbPegs = mastermind_get_nb_pegs_per_turn( mastermind );
	struct Peg const *pegs = mastermind_get_pegs_at_turn( mastermind, turnToDisplay );
	struct Pin const *pins = mastermind_get_pins_at_turn( mastermind, turnToDisplay );
	u8 const playerTurn = mastermind_get_current_turn( mastermind );
	bool const isCurrentTurnDisplayed = ( playerTurn == turnToDisplay );


	draw_row_board( ul, nbPegs, nbPegs );
	draw_row_pegs( SCREENPOS( ul.x + 4, ul.y + 2 ), pegs, nbPegs, isCurrentTurnDisplayed );
	draw_row_pins( ul, nbPegs, pins, nbPegs );
	draw_row_turn( ul, nbPegs, turnToDisplay );
}


static void draw_header_title( screenpos const ul, struct WidgetBoard const *board )
{
	usize const titleSize = 39;
	usize const spacesBetween = ( board->totalBoardWidth - titleSize ) / 2;
	screenpos const ulTitle = SCREENPOS( ul.x + spacesBetween, ul.y + 1); 

	console_color_fg( ConsoleColorFG_YELLOW );

	console_cursor_set_position( ulTitle.y, ulTitle.x );
	console_draw( L"_  _ ___ ___ ___ ___ ___ _  _ _ _  _ __ " );
	console_cursor_set_position( ulTitle.y + 1, ulTitle.x );
	console_draw( L"|\\/| |_| [_   |  |_  |_/ |\\/| | |\\ | | \\" );
	console_cursor_set_position( ulTitle.y + 2, ulTitle.x );
	console_draw( L"|  | | | __]  |  |__ | \\ |  | | | \\| |_/" );
}


static void draw_header_board( screenpos const ul, struct WidgetBoard const *board )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );

	usize const width = board->totalBoardWidth;

	// First line
	console_cursor_set_position( ul.y, ul.x );
	console_draw( L"   " );
	draw_character_n_times( L'#', width - 6 );
	console_draw( L"   " );

	// Second line
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L" ####" );
	draw_character_n_times( L' ', width - 10 );
	console_draw( L"#### " );

	// Third line
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( L" ##" );
	draw_character_n_times( L' ', width - 6 );
	console_draw( L"## " );

	// Fourth line
	console_cursor_set_position( ul.y + 3, ul.x );
	console_draw( L"###" );
	draw_character_n_times( L' ', width - 6 );
	console_draw( L"###" );

	// Fifth line
	console_cursor_set_position( ul.y + 4, ul.x );
	console_draw( L"#####" );
	draw_character_n_times( L' ', width - 10 );
	console_draw( L"#####" );

	// Sixth line
	console_cursor_set_position( ul.y + 5, ul.x );
	draw_character_n_times( L'#', width );

	draw_header_title( ul, board );
}


static void draw_solution_pegs( screenpos const ul, struct WidgetBoard const *board )
{
	usize const pegsSize = board->totalPegSize;
	usize const spacesBetween = ( board->totalBoardWidth - pegsSize ) / 2; // ul.x is not the beginning of the board though, hence the decalage.
	screenpos const ulSolution = SCREENPOS( ul.x + spacesBetween, ul.y + 1 );

	struct Peg const *solution = mastermind_get_solution( mastermind_get_instance() );
	draw_row_pegs( ulSolution, solution, mastermind_get_nb_pegs_per_turn( mastermind_get_instance() ), false );
}


static void draw_footer_board( screenpos const ul, struct WidgetBoard const *board )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );

	usize const width = board->totalBoardWidth;

	// First line
	console_cursor_set_position( ul.y, ul.x );
	console_draw( L"######" );
	draw_character_n_times( L' ', width - 12 );
	console_draw( L"######" );

	// Second line
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L"####" );
	draw_character_n_times( L' ', width - 8 );
	console_draw( L"####" );

	// Thirth line
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( L"###" );
	draw_character_n_times( L' ', width - 6 );
	console_draw( L"###" );

	// Forth line
	console_cursor_set_position( ul.y + 3, ul.x );
	console_draw( L" ###" );
	draw_character_n_times( L' ', width - 8 );
	console_draw( L"### " );

	// Fifth line
	console_cursor_set_position( ul.y + 4, ul.x );
	console_draw( L"  ####" );
	draw_character_n_times( L' ', width - 12 );
	console_draw( L"####  " );

	// Sixth line
	console_cursor_set_position( ul.y + 5, ul.x );
	console_draw( L"    " );
	draw_character_n_times( L'#', width - 8 );
	console_draw( L"    " );

	draw_solution_pegs( ul, board );
}


static void calculate_board_display( struct Widget *widget, struct Mastermind const *mastermind )
{
	struct WidgetBoard *board = (struct WidgetBoard *)widget;

	usize const nbPegs = mastermind_get_nb_pegs_per_turn( mastermind );

	usize const totalWidth = widget->rectBox.size.w;
	board->totalPegSize = nbPegs * ( PEG_WIDTH + PEG_INTERSPACE ) - PEG_INTERSPACE;
	board->totalPinSize = ( ( nbPegs + 1 ) / 2 ) * ( PIN_WIDTH + PIN_INTERSPACE ) - PIN_INTERSPACE;
	usize const baseBoardSize =  4 /*leftBoardPart*/ + 7 /*middle part*/ + 5 /*right side*/;

	board->totalBoardWidth = baseBoardSize + board->totalPegSize + board->totalPinSize;
	board->spacesBetweenBoard = ( totalWidth - board->totalBoardWidth ) / 2;
}


static void redraw_callback( struct Widget *widget )
{
	struct Mastermind const *mastermind = mastermind_get_instance();
	struct WidgetBoard *board = (struct WidgetBoard *)widget;
	screenpos const ul = rect_get_corner( &widget->rectBox, RectCorner_UL );

	int x = ul.x + board->spacesBetweenBoard;
	int y = ul.y;

	if ( board->lastDisplayedTurn == 3 )
	{
		draw_header_board( SCREENPOS( x, y ), board );
	}
	else
	{
		draw_row( SCREENPOS( x, y ), mastermind, board->lastDisplayedTurn - 3 );
	}

	draw_row( SCREENPOS( x, y + ROW_HEIGHT ), mastermind, board->lastDisplayedTurn - 2 );
	draw_row( SCREENPOS( x, y + ROW_HEIGHT * 2 ), mastermind, board->lastDisplayedTurn - 1 );

	if ( board->lastDisplayedTurn == mastermind_get_total_nb_turns( mastermind ) + 1 )
	{
		console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
		console_cursor_set_position( y + ROW_HEIGHT * 3, x );
		draw_character_n_times( L'#', board->totalBoardWidth );
		draw_footer_board( SCREENPOS( x, y + ROW_HEIGHT * 3 + 1 ), board );
	}
	else
	{
		draw_row( SCREENPOS( x, y + ROW_HEIGHT * 3 ), mastermind, board->lastDisplayedTurn );
		console_cursor_set_position( y + ROW_HEIGHT * 4, x );
		console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
		draw_character_n_times( L'#', board->totalBoardWidth );
	}
/*
	y += 1;

	console_cursor_set_position( y + 1, widget->box.contentUpLeft.x );
	console_color_fg( ConsoleColorFG_WHITE );
	console_draw( L"──────────────────────────────────────────────────────────────────────────────────────" );

	y += 1;

	x += 8;

	draw_peg( (screenpos){ .x = x, .y = y + 1 }, ConsoleColorFG_MAGENTA, false );
	draw_peg( (screenpos){ .x = x + 1 + 8, .y = y + 1 }, ConsoleColorFG_BRIGHT_RED, false );
	draw_peg( (screenpos){ .x = x + 2 + 16, .y = y + 1 }, ConsoleColorFG_YELLOW, false );
	draw_peg( (screenpos){ .x = x + 3 + 24, .y = y + 1 }, ConsoleColorFG_BRIGHT_GREEN, false );
	draw_peg( (screenpos){ .x = x + 4 + 32, .y = y + 1 }, ConsoleColorFG_WHITE, false );
	draw_peg( (screenpos){ .x = x + 5 + 40, .y = y + 1 }, ConsoleColorFG_BRIGHT_CYAN, false );
	draw_peg( (screenpos){ .x = x + 6 + 48, .y = y + 1 }, ConsoleColorFG_BRIGHT_BLUE, false );
	draw_peg( (screenpos){ .x = x + 7 + 56, .y = y + 1 }, ConsoleColorFG_BRIGHT_YELLOW, false );*/
}


static void on_game_update_callback( struct Widget *widget, struct Mastermind const *mastermind, enum GameUpdateType type )
{
	if ( type == GameUpdateType_GAME_NEW )
	{		
		calculate_board_display( widget, mastermind );
		((struct WidgetBoard *)widget)->lastDisplayedTurn = 3;
		widget->enabled = true;
		widget->redrawNeeded = true;
	}
	else if ( type == GameUpdateType_GAME_FINISHED )
	{
		widget->redrawNeeded = true;
	}
	else if ( type == GameUpdateType_SELECTION_BAR_MOVED )
	{
		// TODO: Only remove the old selection bar, and display the new one
		widget->redrawNeeded = true;
	}
}


static bool on_input_received_callback( struct Widget *widget, enum KeyInput input )
{
	struct WidgetBoard *board = (struct WidgetBoard *)widget;

	// TODO:
	// How do we disable the history buttons of the button widget if we can't go up anymore ? Same question for the down

	switch( input )
	{
		case KeyInput_D:
		{
			u8 const nbTurns = mastermind_get_total_nb_turns( mastermind_get_instance() );
			if ( board->lastDisplayedTurn < nbTurns + 1 )
			{
				board->lastDisplayedTurn += 1;
				widget->redrawNeeded = true;
			}
			return true;
		}
		
		case KeyInput_U:
		{
			if ( board->lastDisplayedTurn > 3 )
			{
				board->lastDisplayedTurn -= 1;
				widget->redrawNeeded = true;
			}
			return true;
		}
	
	}
	return false;
}


struct Widget *widget_board_create( void )
{
    struct WidgetBoard *const board = malloc( sizeof( struct WidgetBoard ) );
    if ( !board ) return NULL;
	memset( board, 0, sizeof( *board ) );

	struct Widget *const widget = &board->header;

    widget->id = WidgetId_BOARD;
	widget->enabled = false;
	widget->redrawNeeded = false;

	widget->rectBox = rect_make( SCREENPOS( 2, 3 ), VEC2U16( TOTAL_BOARD_SIZE, 25 ) );

	widget->callbacks.redrawCb = redraw_callback;
	widget->callbacks.gameUpdateCb = on_game_update_callback;
	widget->callbacks.inputReceivedCb = on_input_received_callback;
	widget->callbacks.clearCb = NULL;
	widget->callbacks.frameCb = NULL;
	widget->callbacks.resizeCb = NULL;

	board->lastDisplayedTurn = 3; // Title + 1st turn + 2nd turn

	return (struct Widget *)board;
}
