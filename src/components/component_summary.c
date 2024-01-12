#include "components/component_summary.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "characters_list.h"
#include "mastermind.h"
#include "game.h"

struct ComponentSummary
{
	struct ComponentHeader header; /* TODO ComponentHeader instead of Widget */

	// Component Specific Data
	struct Rect box;
	screenpos firstPegsRowUL;
	screenpos firstTurnRowUL;
	screenpos firstPinsRowUL;
	screenpos solutionRowUL;
};
#define CAST_TO_COMPONENT( _header ) ( ( struct ComponentSummary * )( _header ) )


static void draw_pin( struct Pin const *pin, bool visible )
{
	if ( !visible )
	{
		console_draw( L" " );
		return;
	}

	console_color_fg( pin_get_color( pin->id ) );
	utf16 const character = pin->id == PinId_INCORRECT ? UTF16C_SmallDottedCircle : UTF16C_SmallFilledCircle;
	console_draw( L"%lc", character );
}


static void draw_pegs_at_turn( struct ComponentSummary const *comp, usize const turn )
{
	screenpos const ul = SCREENPOS( comp->firstPegsRowUL.x, comp->firstPegsRowUL.y + ( turn - 1 ) );
	struct Peg const *pegs      = mastermind_get_pegs_at_turn( turn );
	usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();

	for ( usize idx = 0; idx < nbPiecesPerTurn; ++idx )
	{
		peg_draw_single_character( &pegs[idx], ul.x + ( idx * 2 ), ul.y );
	}
}


static void draw_current_turn_nb_at_turn( struct ComponentSummary const *comp, usize const turn )
{
	usize const playerTurn    = mastermind_get_player_turn();
	bool const isGameFinished = mastermind_is_game_finished();

	if ( !isGameFinished && turn <= playerTurn )
	{
		bool const isPlayerTurn = ( playerTurn == turn );
		console_color_fg( isPlayerTurn ? ConsoleColorFG_YELLOW : ConsoleColorFG_WHITE );
	}
	else
	{
		console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
	}

	screenpos const ul = SCREENPOS( comp->firstTurnRowUL.x, comp->firstTurnRowUL.y + ( turn - 1 ) );
	console_cursor_setpos( ul );
	console_draw( L"%02u", turn );
}


static void draw_pins_at_turn( struct ComponentSummary const *comp, usize const turn )
{
	screenpos const ul = SCREENPOS( comp->firstPinsRowUL.x, comp->firstPinsRowUL.y + ( turn - 1 ) );
	struct Pin const *pins      = mastermind_get_pins_at_turn( turn );
	usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();
	usize const playerTurn      = mastermind_get_player_turn();
	bool const pinVisible       = mastermind_is_game_finished() || turn < playerTurn;

	console_cursor_setpos( ul );
	for ( usize idx = 0; idx < nbPiecesPerTurn; ++idx )
	{
		draw_pin( &pins[idx], pinVisible );
	}
}


static void redraw_callback( struct ComponentHeader *widget )
{
	struct ComponentSummary const *comp = CAST_TO_COMPONENT( widget );

	rect_draw_borders( &comp->box, L"Summary" );

	struct Mastermind const *mastermind = mastermind_get_instance();
    usize const nbTurns = mastermind_get_total_turns();
    usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();
	usize const currTurn = mastermind_get_player_turn();
	bool const isGameFinished = mastermind_is_game_finished();

	screenpos const ul = rect_get_corner( &comp->box, RectCorner_UL );
	usize const width = comp->box.size.w;

	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

    for ( int y = 0; y < nbTurns; ++y )
    {
		usize const displayTurn = y + 1;
		draw_pegs_at_turn( comp, displayTurn );
		draw_current_turn_nb_at_turn( comp, displayTurn );
		draw_pins_at_turn( comp, displayTurn );

		// That's a crappy way of doing it now, so we would need to cleanup that by encapsulating that "darker" color in console.h
		if ( !isGameFinished && y + 1 == currTurn ) { console_draw( L"\x1B[2m" ); }
    }
	console_color_reset();

	struct Peg const *solution = mastermind_get_solution();
	screenpos const solutionPos = comp->solutionRowUL;
    for ( usize x = 0; x < nbPiecesPerTurn; ++x )
    {
		peg_draw_single_character( &solution[x], solutionPos.x + 2 * x, solutionPos.y );
    }

	console_color_reset();
}

// TODO Move it in another file if the usage is approved.
static inline screenpos screenpos_add( screenpos const lhs, screenpos const rhs )
{
	return SCREENPOS( lhs.x + rhs.x, lhs.y + rhs.y );
}


static void set_component_data( struct ComponentSummary *const comp )
{
    usize const nbTurns = mastermind_get_total_turns();
    usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();

	vec2u16 const boxSize = (vec2u16) {
		.x = 4 /*borders + space each side*/ + ( nbPiecesPerTurn * 2 ) - 1 /*pegs*/ + 4 /*turn display*/ + nbPiecesPerTurn /*pins*/,
		.y = 2 /*borders*/ + nbTurns + 1 /*solution*/
	};
	// We want to keep the board on the right of the screen, whether we have 4 or 6 pegs to display.
	screenpos const boxUL = SCREENPOS( GAME_SIZE_WIDTH - 1 - boxSize.w, 6 );

	usize const spacesBeforeSolution = ( boxSize.w - ( ( nbPiecesPerTurn * 2 ) - 1 ) ) / 2;

	comp->box = rect_make( boxUL, boxSize );
	comp->firstPegsRowUL = SCREENPOS( boxUL.x + 2, boxUL.y + 1 );
	comp->firstTurnRowUL = screenpos_add( comp->firstPegsRowUL, SCREENPOS( ( nbPiecesPerTurn * 2 ), 0 ) );
	comp->firstPinsRowUL = screenpos_add( comp->firstTurnRowUL, SCREENPOS( 3, 0 ) );
	comp->solutionRowUL = SCREENPOS(  boxUL.x + spacesBeforeSolution, boxUL.y + ( boxSize.y - 2 ) );
}


static void on_game_update_callback( struct ComponentHeader *widget, struct Mastermind const *mastermind, enum GameUpdateType type )
{
	if ( type == GameUpdateType_GAME_NEW )
	{
		set_component_data( CAST_TO_COMPONENT( widget ) );
		widget->refreshNeeded = true;
		widget->enabled = true;
	}
	else if ( type == GameUpdateType_GAME_FINISHED )
	{
		widget->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_NEXT_TURN )
	{
		widget->refreshNeeded = true;
	}
	else if ( type == GameUpdateType_PEG_ADDED )
	{
		widget->refreshNeeded = true;
	}

	// TODO to catch:
	// Reset turn
	// Peg removed from the board
}


struct ComponentHeader *component_summary_create( void )
{
    struct ComponentSummary *const comp = calloc( 1, sizeof( struct ComponentSummary ) );
    if ( !comp ) return NULL;

	component_make_header( &comp->header, ComponentId_SUMMARY, false );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->redrawCb = redraw_callback;
	callbacks->gameUpdateCb = on_game_update_callback;

	return (struct ComponentHeader *)comp;
}
