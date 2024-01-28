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
    usize nbTurns;

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


/*static void draw_current_turn_nb_at( struct WidgetGameSummary *widget, usize const turn )
{
    if ( widget->currTurn > turn )
    {
    	style_update( STYLE( FGColor_WHITE ) );
    }
    else if ( widget->currTurn == turn )
    {
    	style_update( STYLE( FGColor_YELLOW ) );
    }
    else
    {
		style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
    }

	screenpos const ul = SCREENPOS( widget->firstTurnRowUL.x, widget->firstTurnRowUL.y + ( turn - 1 ) );
	cursor_update_pos( ul );
	term_write( L"%02u", turn );
}*/


static void draw_pin_at( struct WidgetGameSummary *widget, usize const turn, usize const index, gamepiece const piece )
{
	screenpos ul = SCREENPOS( widget->firstPinsRowUL.x, widget->firstPinsRowUL.y + ( turn - 1 ) );
    ul.x += ( index * 1 );

	piece_write_1x1( ul, piece );
}


static void draw_solution_at( struct WidgetGameSummary *widget, usize const index, gamepiece const piece )
{
	screenpos solutionPos = widget->solutionRowUL;
    solutionPos.x += ( index * 2 );

    piece_write_1x1( solutionPos, piece );
}


static void draw_solution( struct WidgetGameSummary *widget, gamepiece const *solution )
{
	screenpos solutionPos = widget->solutionRowUL;

    for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
    {
		piece_write_1x1( solutionPos, solution[x] );
		solutionPos.x += 2;
    }
}


static void draw_turn_at( struct WidgetGameSummary *widget, usize const turn )
{
    	screenpos const ul = SCREENPOS( widget->firstTurnRowUL.x, widget->firstTurnRowUL.y + turn - 1 );
    	cursor_update_pos( ul );
	    term_write( L"%02u", turn );
}


static void draw_turns( struct WidgetGameSummary *widget )
{
    style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
    for ( int y = 0; y < widget->nbTurns; ++y )
    {
        draw_turn_at( widget, y + 1 );
    }
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetGameSummary *widget = (struct WidgetGameSummary *)subscriber;

    switch ( event->type )
    {
        case EventType_GAME_NEW:
        {
            if ( widget->nbPegsPerTurn != event->newGame.nbPegsPerTurn || widget->nbTurns != event->newGame.nbTurns )
            {
                rect_clear_content( &widget->box );
                widget->nbPegsPerTurn = event->newGame.nbPegsPerTurn;
                widget->nbTurns = event->newGame.nbTurns;
            }
            init_widget_data( widget );
            draw_turns( widget );
            break;
        }

        case EventType_PEG_ADDED:
        case EventType_PEG_REMOVED:
        case EventType_PEG_HIDDEN:
        case EventType_PEG_REVEALED:
        {
            struct EventPeg const *evPeg = &event->peg;
            if ( evPeg->turn == Mastermind_SOLUTION_TURN )
            {
                draw_solution_at( widget, evPeg->index, evPeg->piece );
            }
            else
            {
                draw_peg_at( widget, evPeg->turn, evPeg->index, evPeg->piece );
            }
            break;
        }

        case EventType_PIN_ADDED:
        case EventType_PIN_REMOVED:
        {
            struct EventPin const *evPin = &event->pin;
            draw_pin_at( widget, evPin->turn, evPin->index, evPin->piece );
            break;
        }

        case EventType_NEW_TURN:
        {
            usize newTurn = event->newTurn.turn;
            if ( newTurn > 1 )
            {
                style_update( STYLE( FGColor_WHITE ) );
                draw_turn_at( widget, newTurn - 1 );
            }
            style_update( STYLE( FGColor_YELLOW ) );
            draw_turn_at( widget, newTurn );
            break;
        }

    // turn next / reset
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
    event_subscribe( widget, EventType_MaskAll );

    return (struct Widget *)widget;
}