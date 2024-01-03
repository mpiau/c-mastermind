#include "widgets/widget_board.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "console.h"

struct WidgetBoard
{
	struct Widget header;

	u8 lastDisplayedTurn;
};


enum // Constants
{
	PEG_WIDTH = 6,
	PEG_INTERSPACE = 2,
	PIN_WIDTH = 4,
	PIN_INTERSPACE = 1,
	ROW_HEIGHT = 6,
};


static void draw_peg( screenpos const ul, enum PegId const id, bool const hidden )
{
	bool const isEmpty = ( id == PegId_Empty );

	if ( hidden )
	{
		console_draw( L"\x1B[2m" );
		console_color_fg( ConsoleColorBG_BRIGHT_BLACK );
		console_draw( L",d||b." );
		console_cursor_set_position( ul.y + 1, ul.x );
		console_draw( L"O ?? O" );
		console_cursor_set_position( ul.y + 2, ul.x );
		console_draw( L"`Y||P'" );
		console_color_reset();
	}
	else
	{
		console_color_fg( peg_get_color( id, false /* TODO be relevant to the board state */ ) );
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


static void draw_row_pegs( screenpos const ul, struct Peg const *pegs, u32 const nbPegs )
{
	for ( u32 pegIdx = 0; pegIdx < nbPegs; ++pegIdx )
	{
		screenpos const pegUL = (screenpos) {
			.x = ul.x + 4 + ( pegIdx * ( PEG_WIDTH + PEG_INTERSPACE ) ),
			.y = ul.y + 2
		};
		draw_peg( pegUL, pegs[pegIdx].id, pegs[pegIdx].hidden );
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
	u8 nbPegs = mastermind_get_nb_pegs_per_turn( mastermind );
	struct Peg const *pegs = mastermind_get_pegs_at_turn( mastermind, turnToDisplay );
	struct Pin const *pins = mastermind_get_pins_at_turn( mastermind, turnToDisplay );

	draw_row_board( ul, nbPegs, nbPegs );
	draw_row_pegs( ul, pegs, nbPegs );
	draw_row_pins( ul, nbPegs, pins, nbPegs );
	draw_row_turn( ul, nbPegs, turnToDisplay );
}


static void draw_header_board( screenpos const ul )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );

	// TODO needs to adjust depending of the number of pegs

	console_cursor_set_position( ul.y, ul.x );
	console_draw( L"   #################################################   " );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L" ####                                             #### " );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( L" ##                                                 ## " );
	console_cursor_set_position( ul.y + 3, ul.x );
	console_draw( L"###                                                 ###" );
	console_cursor_set_position( ul.y + 4, ul.x );
	console_draw( L"#####                                             #####" );
	console_cursor_set_position( ul.y + 5, ul.x );
	console_draw( L"#######################################################" );
}


static void draw_header_title( screenpos const ul, u16 const nbPegs )
{
}


static void redraw_callback( struct Widget *widget )
{
	struct Mastermind const *mastermind = mastermind_get_instance();
	struct WidgetBoard *board = (struct WidgetBoard *)widget;
	screenpos const ul = rect_get_corner( &widget->rectBox, RectCorner_UL );

	int x = ul.x + 2;
	int y = ul.y + 1;

//	draw_header_board( (screenpos){ x, y } );
//	draw_header_title( (screenpos){ x, y }, 3 );
/*	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
	console_cursor_set_position( y++, x );
	console_draw( L"   ##############################################   " );
	console_cursor_set_position( y++, x );
	console_draw( L" #### %S_  _ ___ ___ ___ ___ ___ _  _ _ _  _ __  %S#### ", L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L" ##   %S|\\/| |_| [_   |  |_  |_/ |\\/| | |\\ | | \\   %S## ",  L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L"###   %S|  | | | __]  |  |__ | \\ |  | | | \\| |_/   %S###",  L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L"#####                                          #####" );
	console_cursor_set_position( y++, x );
	console_draw( L"####################################################" );
	console_draw( L"####################################################" );
	draw_row( (screenpos){ .x = x, .y = y } );*/

	if ( board->lastDisplayedTurn == 3 ) draw_header_board( SCREENPOS( x, y ) );
	else draw_row( SCREENPOS( x, y ), mastermind, board->lastDisplayedTurn - 3 );

	draw_row( SCREENPOS( x, y + ROW_HEIGHT ), mastermind, board->lastDisplayedTurn - 2 );
	draw_row( SCREENPOS( x, y + ROW_HEIGHT * 2 ), mastermind, board->lastDisplayedTurn - 1 );

	if ( board->lastDisplayedTurn == mastermind_get_total_nb_turns( mastermind ) + 1 ) draw_header_board( SCREENPOS( x, y + ROW_HEIGHT * 3 ) );
	else draw_row( SCREENPOS( x, y + ROW_HEIGHT * 3 ), mastermind, board->lastDisplayedTurn );

	y = y + ROW_HEIGHT * 4;
	x = x - 2;
	console_cursor_set_position( y, ul.x + 2 );
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
	console_draw( L"############################################################################" );
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
	widget->enabled = true;
	widget->redrawNeeded = true;
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

	widget->rectBox = rect_make( SCREENPOS( 2, 2 ), VEC2U16( 86, 25 ) );

	widget->callbacks.redrawCb = redraw_callback;
	widget->callbacks.gameUpdateCb = on_game_update_callback;
	widget->callbacks.inputReceivedCb = on_input_received_callback;
	widget->callbacks.clearCb = NULL;
	widget->callbacks.frameCb = NULL;
	widget->callbacks.resizeCb = NULL;

	board->lastDisplayedTurn = 3; // Title + 1st turn + 2nd turn

	return (struct Widget *)board;
}
