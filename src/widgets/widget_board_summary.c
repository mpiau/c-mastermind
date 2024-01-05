#include "widgets/widget_board_summary.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "characters_list.h"
#include "mastermind.h"
#include "game.h"

struct ComponentSummary
{
	struct Widget header; /* TODO ComponentHeader instead of Widget */

	// Component Specific Data
	struct Rect box;
	screenpos firstPegsRowUL;
	screenpos firstTurnRowUL;
	screenpos firstPinsRowUL;
	screenpos solutionRowUL;
};

#define CAST_TO_COMPONENT( _header ) ( struct ComponentSummary * )( _header )


static void draw_peg( struct Peg const *peg )
{
	console_color_fg( peg->hidden ? ConsoleColorFG_BRIGHT_BLACK : peg_get_color( peg->id, false ) );

	utf16 const character = peg->hidden ? UTF16C_QuestionMark : ( peg->id == PegId_Empty ? UTF16C_SmallDottedCircle : UTF16C_BigFilledCircle );
	console_draw( L"%lc", character );
}

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


static void draw_pegs_row( screenpos const ul, usize const turn )
{
	struct Peg const *pegs = mastermind_get_pegs_at_turn( turn );
	usize const nbPegsPerTurn = mastermind_get_nb_pegs_per_turn();

	for ( usize idx = 0; idx < nbPegsPerTurn; ++idx )
	{
		peg_draw_single_character( &pegs[idx], ul.x + ( idx * 2 ), ul.y );
	}
}


static void draw_turn( screenpos const ul, usize const turn )
{
	usize const playerTurn = mastermind_get_player_turn();
	bool const isGameFinished = mastermind_is_game_finished();

	console_cursor_setpos( ul );
	if ( !isGameFinished && turn <= playerTurn )
	{
		if (turn == playerTurn )
		{
			console_color_fg( ConsoleColorFG_YELLOW );
		}
		else
		{
			console_color_fg( ConsoleColorFG_WHITE );
		}
	}
	else
	{
		console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
	}

	console_draw( L"%02u", turn );
}


static void draw_pins_row( screenpos const ul, usize const turn )
{
	struct Pin const *pins = mastermind_get_pins_at_turn( turn );
	usize const nbPegsPerTurn = mastermind_get_nb_pegs_per_turn();
	usize const playerTurn = mastermind_get_player_turn();
	bool const visible = mastermind_is_game_finished() || turn < playerTurn;

	console_cursor_setpos( ul );

	for ( usize idx = 0; idx < nbPegsPerTurn; ++idx )
	{
		draw_pin( &pins[idx], visible );
	}
}


static void redraw_callback( struct Widget *widget )
{
	struct ComponentSummary const *component = CAST_TO_COMPONENT( widget );

	rect_draw_borders( &component->box, L"Summary" );

	struct Mastermind const *mastermind = mastermind_get_instance();
    usize const nbTurns = mastermind_get_total_turns();
    usize const nbPegsPerTurn = mastermind_get_nb_pegs_per_turn();
	usize const currTurn = mastermind_get_player_turn();
	bool const isGameFinished = mastermind_is_game_finished();

	screenpos const ul = rect_get_corner( &component->box, RectCorner_UL );
	usize const width = component->box.size.w;

	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

    for ( int y = 0; y < nbTurns; ++y )
    {
		screenpos const pegsPos = SCREENPOS( component->firstPegsRowUL.x, component->firstPegsRowUL.y + y );
		draw_pegs_row( pegsPos, y + 1 );

		screenpos const turnPos = SCREENPOS( component->firstTurnRowUL.x, component->firstTurnRowUL.y + y );
		draw_turn( turnPos, y + 1 );

		if ( !isGameFinished && y + 1 == currTurn ) { console_draw( L"\x1B[2m" ); }

		screenpos const pinsPos = SCREENPOS( component->firstPinsRowUL.x, component->firstPinsRowUL.y + y );
		draw_pins_row( pinsPos, y + 1 );
    }
	console_color_reset();

	struct Peg const *solution = mastermind_get_solution();
	screenpos const solutionPos = component->solutionRowUL;
    for ( usize x = 0; x < nbPegsPerTurn; ++x )
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


static void set_component_data( struct ComponentSummary *const component )
{
    u32 const nbTurns = mastermind_get_total_turns();
    u32 const nbPegsPerTurn = mastermind_get_nb_pegs_per_turn();

	vec2u16 const boxSize = (vec2u16) {
		.x = 4 /*borders + space each side*/ + ( nbPegsPerTurn * 2 ) - 1 /*pegs*/ + 4 /*turn display*/ + nbPegsPerTurn /*pins*/,
		.y = 2 /*borders*/ + nbTurns + 1 /*solution*/
	};
	// We want to keep the board on the right of the screen, whether we have 4 or 6 pegs to display.
	screenpos const boxUL = SCREENPOS( GAME_SIZE_WIDTH - 1 - boxSize.w, 6 );

	usize const spacesBeforeSolution = ( boxSize.w - ( ( nbPegsPerTurn * 2 ) - 1 ) ) / 2;

	component->box = rect_make( boxUL, boxSize );
	component->firstPegsRowUL = SCREENPOS( boxUL.x + 2, boxUL.y + 1 );
	component->firstTurnRowUL = screenpos_add( component->firstPegsRowUL, SCREENPOS( ( nbPegsPerTurn * 2 ), 0 ) );
	component->firstPinsRowUL = screenpos_add( component->firstTurnRowUL, SCREENPOS( 3, 0 ) );
	component->solutionRowUL = SCREENPOS(  boxUL.x + spacesBeforeSolution, boxUL.y + ( boxSize.y - 2 ) );
}


static void on_game_update_callback( struct Widget *widget, struct Mastermind const *mastermind, enum GameUpdateType type )
{
	if ( type == GameUpdateType_GAME_NEW )
	{
		set_component_data( CAST_TO_COMPONENT( widget ) );
		widget->redrawNeeded = true;
		widget->enabled = true;
	}
	else if ( type == GameUpdateType_GAME_FINISHED )
	{
		widget->redrawNeeded = true;
	}
	else if ( type == GameUpdateType_NEXT_TURN )
	{
		widget->redrawNeeded = true;
	}
	else if ( type == GameUpdateType_PEG_ADDED )
	{
		widget->redrawNeeded = true;
	}

	// TODO to catch:
	// Reset turn
	// Peg removed from the board
}


struct Widget *component_summary_create( void )
{
    struct ComponentSummary *const component = calloc( 1, sizeof( struct ComponentSummary ) );
    if ( !component )
	{
		// Allocation problem, that's pretty bad. In the future, we could have a memoryPool that handle heap alloc.
		return NULL;
	}

	widget_set_header( &component->header, WidgetId_SUMMARY, false );

    struct WidgetCallbacks *const callbacks = &component->header.callbacks;
    callbacks->redrawCb = redraw_callback;
	callbacks->gameUpdateCb = on_game_update_callback;

	return (struct Widget *)component;
}
