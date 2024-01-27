#include "ui/widgets.h"
#include "terminal/terminal.h"
#include "rect.h"
#include "mastermind.h"
#include "game.h"
#include "events.h"

#include <stdlib.h>


struct WidgetGameSummary
{
    struct Widget base;

    usize nbPegsPerTurn;

   	// Component Specific Data
	struct Rect box;
	screenpos firstPegsRowUL;
	screenpos firstTurnRowUL;
	screenpos firstPinsRowUL;
	screenpos solutionRowUL;
};


static void init_widget_data( struct WidgetGameSummary *widget )
{
	usize const nbMissingPieces = Mastermind_MAX_PIECES_PER_TURN - widget->nbPegsPerTurn;
	usize const spacesBeforeSolution = ( widget->box.size.w - 11 ) / 2;
    screenpos const boxUL = rect_get_ul_corner( &widget->box );

	widget->firstPegsRowUL = SCREENPOS( boxUL.x + 2 + nbMissingPieces, boxUL.y + 1 );
	widget->firstTurnRowUL = SCREENPOS( widget->firstPegsRowUL.x + 12 - nbMissingPieces, boxUL.y + 1 );
	widget->firstPinsRowUL = SCREENPOS( widget->firstTurnRowUL.x + 3 + nbMissingPieces / 2, boxUL.y + 1 );
	widget->solutionRowUL  = SCREENPOS( boxUL.x + spacesBeforeSolution + nbMissingPieces, boxUL.y + ( widget->box.size.y - 2 ) );
}


static void draw_peg_at( struct WidgetGameSummary const *widget, usize const turn, usize const index, gamepiece const piece )
{   
	screenpos const ul = (screenpos) {
        .x = widget->firstPegsRowUL.x + ( 2 * index ),
        .y = widget->firstPegsRowUL.y + ( turn - 1 )
    };
    piece_write_1x1( ul, piece );
}


static void draw_pegs_at_turn( struct WidgetGameSummary *widget, usize const turn )
{
	for ( usize idx = 0; idx < widget->nbPegsPerTurn; ++idx )
	{
        draw_peg_at( widget, turn, idx, mastermind_get_peg( turn, idx ) );
	}
}


static void draw_current_turn_nb_at_turn( struct WidgetGameSummary *widget, usize const turn )
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

	screenpos const ul = SCREENPOS( widget->firstTurnRowUL.x, widget->firstTurnRowUL.y + ( turn - 1 ) );
	cursor_update_pos( ul );
	term_write( L"%02u", turn );
}


static void draw_pins_at_turn( struct WidgetGameSummary *widget, usize const turn )
{
	screenpos ul = SCREENPOS( widget->firstPinsRowUL.x, widget->firstPinsRowUL.y + ( turn - 1 ) );

	for ( usize idx = 0; idx < widget->nbPegsPerTurn; ++idx )
	{
		piece_write_1x1( ul, mastermind_get_pin( turn, idx ) );
		ul.x += 1;
	}
}


static void draw_solution( struct WidgetGameSummary *widget )
{
	gamepiece const *solution = mastermind_get_solution();
	screenpos solutionPos = widget->solutionRowUL;

    for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
    {
		piece_write_1x1( solutionPos, solution[x] );
		solutionPos.x += 2;
    }
}


static void draw_summary_turn( struct WidgetGameSummary *widget, usize const turn )
{
	draw_pegs_at_turn( widget, turn );
	draw_current_turn_nb_at_turn( widget, turn );
	draw_pins_at_turn( widget, turn );
} 


static void draw_summary( struct WidgetGameSummary *widget )
{
    usize const nbTurns = mastermind_get_total_turns();
    for ( int y = 0; y < nbTurns; ++y )
    {
		draw_summary_turn( widget, y + 1 );
    }
	draw_solution( widget );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetGameSummary *widget = (struct WidgetGameSummary *)subscriber;

    switch ( event->type )
    {
        case EventType_GAME_NEW:
        {
            if ( widget->nbPegsPerTurn != event->newGame.nbPegsPerTurn )
            {
                rect_clear_content( &widget->box );
                widget->nbPegsPerTurn = event->newGame.nbPegsPerTurn;
            }
            init_widget_data( widget );
            draw_summary( widget );
            break;
        }

        case EventType_GAME_LOST:
        case EventType_GAME_WON:
        {
//            draw_pins_at_turn( widget, mastermind_get_player_turn() );
//            write_solution( widget );
            break;
        }
        case EventType_PEG_ADDED:
        {
            struct EventPegAdded const *evData = &event->pegAdded;
            draw_peg_at( widget, evData->turn, evData->index, evData->piece );
            break;
        }
        case EventType_PEG_REMOVED:
        {
            struct EventPegRemoved const *evData = &event->pegRemoved;
            draw_peg_at( widget, evData->turn, evData->index, evData->piece );
        }
    // peg added / removed
    // turn next / reset
    // Solution Reveal / hide
    };

    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
	struct WidgetGameSummary *widget = (struct WidgetGameSummary *)base;
	rect_draw_borders( &widget->box, L"Summary" );
}


static void disable_callback( struct Widget *base )
{
    rect_clear( &( (struct WidgetGameSummary *)base )->box );
}


struct Widget *widget_game_summary_create( void )
{
    struct WidgetGameSummary *const widget = calloc( 1, sizeof( struct WidgetGameSummary ) );
    if ( !widget ) return NULL;

    widget->base.name = "GameSummary";
    widget->base.enabledScenes = UIScene_IN_GAME;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    // Widget specific
    
	// We want to keep the board on the right of the screen, whether we have 4 or 6 pegs to display.
	vec2u16 const boxSize = (vec2u16) { .x = 25, .y = 24 };
	screenpos const boxUL = SCREENPOS( GAME_SIZE_WIDTH - boxSize.w, 5 );
	widget->box = rect_make( boxUL, boxSize );

    event_register( widget, on_event_callback );
    event_subscribe( widget, EventType_GAME_NEW | EventType_GAME_LOST | EventType_GAME_WON | EventType_PEG_ADDED | EventType_PEG_REMOVED/* | [...] */ );

    return (struct Widget *)widget;
}