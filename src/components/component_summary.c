#include "components/component_summary.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "characters_list.h"
#include "mastermind.h"
#include "game.h"
#include "rect.h"

#include <stdlib.h>


struct ComponentSummary
{
	struct ComponentHeader header;

	// Component Specific Data
	struct Rect box;
	screenpos firstPegsRowUL;
	screenpos firstTurnRowUL;
	screenpos firstPinsRowUL;
	screenpos solutionRowUL;
};
#define CAST_TO_COMP( _header ) ( ( struct ComponentSummary * )( _header ) )


static void draw_pegs_at_turn( struct ComponentSummary const *comp, usize const turn )
{
	usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();
	screenpos ul = SCREENPOS( comp->firstPegsRowUL.x, comp->firstPegsRowUL.y + ( turn - 1 ) );

	for ( usize idx = 0; idx < nbPiecesPerTurn; ++idx )
	{
		piece_write_1x1( ul, mastermind_get_peg( turn, idx ) );
		ul.x += 2;
	}
}


static void draw_current_turn_nb_at_turn( struct ComponentSummary const *comp, usize const turn )
{
	enum PieceTurn const turnStatus = ( mastermind_get_peg( turn, 0 ) & PieceTurn_MaskAll );

	switch( turnStatus )
	{
		case PieceTurn_PAST:
			style_update( STYLE( FGColor_WHITE ) );
			break;
		case PieceTurn_CURRENT:
			style_update( STYLE( FGColor_YELLOW ) );
			break;
		case PieceTurn_FUTURE:
			style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
			break;

		case PieceTurn_IGNORE:
		default:
			style_update( STYLE( FGColor_BRIGHT_BLACK ) );
			break;
	}

	screenpos const ul = SCREENPOS( comp->firstTurnRowUL.x, comp->firstTurnRowUL.y + ( turn - 1 ) );
	cursor_update_pos( ul );
	term_write( L"%02u", turn );
}


static void draw_pins_at_turn( struct ComponentSummary const *comp, usize const turn )
{
	screenpos ul = SCREENPOS( comp->firstPinsRowUL.x, comp->firstPinsRowUL.y + ( turn - 1 ) );
	usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();

	for ( usize idx = 0; idx < nbPiecesPerTurn; ++idx )
	{
		piece_write_1x1( ul, mastermind_get_pin( turn, idx ) );
		ul.x += 1;
	}
}


static void write_turn( struct ComponentSummary const *comp, usize const turn )
{
	draw_pegs_at_turn( comp, turn );
	draw_current_turn_nb_at_turn( comp, turn );
	draw_pins_at_turn( comp, turn );
} 


static void write_solution( struct ComponentSummary const *comp )
{
    usize const nbPiecesPerTurn = mastermind_get_nb_pieces_per_turn();
	gamepiece const *solution = mastermind_get_solution();
	screenpos solutionPos = comp->solutionRowUL;

    for ( usize x = 0; x < nbPiecesPerTurn; ++x )
    {
		piece_write_1x1( solutionPos, solution[x] );
		solutionPos.x += 2;
    }
}


static void write_board_content( struct ComponentSummary const *comp )
{
    usize const nbTurns = mastermind_get_total_turns();
    for ( int y = 0; y < nbTurns; ++y )
    {
		write_turn( comp, y + 1 );
    }
	write_solution( comp );
}


// TODO Move it in another file if the usage is approved.
static inline screenpos screenpos_add( screenpos const lhs, screenpos const rhs )
{
	return SCREENPOS( lhs.x + rhs.x, lhs.y + rhs.y );
}


static void set_component_data( struct ComponentSummary *const comp )
{
	vec2u16 const boxSize = (vec2u16) {
		.x = 25 ,
		.y = 24
	};
	// We want to keep the board on the right of the screen, whether we have 4 or 6 pegs to display.
	screenpos const boxUL = SCREENPOS( GAME_SIZE_WIDTH - boxSize.w, 5 );

	usize const spacesBeforeSolution = ( boxSize.w - 11 ) / 2;

	usize const nbMissingPieces = Mastermind_MAX_PIECES_PER_TURN - mastermind_get_nb_pieces_per_turn();

	comp->box = rect_make( boxUL, boxSize );
	comp->firstPegsRowUL = SCREENPOS( boxUL.x + 2 + nbMissingPieces, boxUL.y + 1 );
	comp->firstTurnRowUL = screenpos_add( comp->firstPegsRowUL, SCREENPOS( 12 - nbMissingPieces, 0 ) );
	comp->firstPinsRowUL = screenpos_add( comp->firstTurnRowUL, SCREENPOS( 3 + nbMissingPieces / 2, 0 ) );
	comp->solutionRowUL = SCREENPOS(  boxUL.x + spacesBeforeSolution + nbMissingPieces, boxUL.y + ( boxSize.y - 2 ) );
}


static void event_received_callback( struct ComponentHeader *header, enum EventType event, struct EventData const *data )
{
	struct ComponentSummary *comp = CAST_TO_COMP( header );

	if ( event == EventType_GAME_NEW )
	{
		set_component_data( comp );
		write_board_content( comp );
	}
	else if ( event == EventType_GAME_LOST || event == EventType_GAME_WON )
	{
		draw_pins_at_turn( comp, mastermind_get_player_turn() );
		write_solution( comp );
	}
	else if ( event == EventType_NEXT_TURN )
	{
		usize const currTurn = mastermind_get_player_turn();
		write_turn( comp, currTurn - 1 );
		write_turn( comp, currTurn );
	}
	else if ( event == EventType_PEG_ADDED || event == EventType_PEG_REMOVED )
	{
		draw_pegs_at_turn( comp, mastermind_get_player_turn() );
	}
}


static void enable_callback( struct ComponentHeader *header )
{
	struct ComponentSummary *comp = CAST_TO_COMP( header );
	set_component_data( comp );
	rect_draw_borders( &comp->box, L"Summary" );
	write_board_content( comp );
}


static void disable_callback( struct ComponentHeader *header )
{
	rect_clear( &CAST_TO_COMP( header )->box );
}


struct ComponentHeader *component_summary_create( void )
{
    struct ComponentSummary *const comp = calloc( 1, sizeof( struct ComponentSummary ) );
    if ( !comp ) return NULL;

	comp->header.id = ComponentId_SUMMARY;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
	callbacks->enableCb = enable_callback;
	callbacks->disableCb = disable_callback;
	callbacks->eventReceivedCb = event_received_callback;

	return (struct ComponentHeader *)comp;
}
