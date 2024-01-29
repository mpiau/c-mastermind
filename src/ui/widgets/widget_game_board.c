#include "ui/widgets.h"
#include "rect.h"
#include "ui.h"
#include "events.h"
#include "requests.h"
#include "keybindings.h"
#include "mastermind.h"

#include <stdlib.h>


enum // Constants
{
    TOTAL_BOARD_WIDTH = 78,
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
	struct Rect pegs[Mastermind_MAX_PIECES_PER_TURN];
	struct Rect pins[Mastermind_MAX_PIECES_PER_TURN];
	struct Rect turn;
};

struct WidgetGameBoard
{
    struct Widget base;

	struct Rect box;
    usize nbPegsPerTurn;
    usize nbTurns;
    usize lastDispTurn;
    struct BoardRow rows[ROWS_DISPLAYED];
    struct Rect solution[Mastermind_MAX_PIECES_PER_TURN];
    u64 buttons[ButtonIdx_Count];
};


static inline void draw_character_n_times( utf16 const character, usize const n )
{
	for ( usize x = 0; x < n; ++x )
	{
		term_write( L"%lc", character );
	}
}


static void draw_internal_board_lines( struct WidgetGameBoard *widget )
{
	screenpos const ul = rect_get_ul_corner( &widget->box );

	// Lines separating the turns
	style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
	for( usize idx = 0; idx < 3; idx++ )
	{
		cursor_update_yx( ul.y + 6 * ( idx + 1 ), ul.x + 2 ); 
		draw_character_n_times( L'─', TOTAL_BOARD_WIDTH - 4 );
	}

	// Line that separates buttons from the rest of the game
	cursor_update_yx( ul.y + widget->box.size.h - 3, ul.x );
	style_update( STYLE( FGColor_BRIGHT_BLACK ) );
	term_write( L"%lc", L'├' );
	draw_character_n_times( L'─', TOTAL_BOARD_WIDTH - 2 );
	term_write( L"%lc", L'┤' );
}


static void init_widget_data( struct WidgetGameBoard *widget )
{
    widget->lastDispTurn = 4;

	screenpos const ul = rect_get_ul_corner( &widget->box );
	for ( usize y = 0; y < ROWS_DISPLAYED; ++y )
	{
        struct BoardRow *row = &widget->rows[y];
		for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
		{
            struct Rect *rect = &row->pegs[x];
			screenpos const pos = {
				.x = ul.x + 3 + 8 * x + ( ( Mastermind_MAX_PIECES_PER_TURN - widget->nbPegsPerTurn ) * 2 ),
				.y = ul.y + 2 + 6 * y
			};
			*rect = rect_make( pos, VEC2U16( 6, 3 ) );
        }

   		// Turn
		{
			screenpos const lastPegBR = rect_get_br_corner( &row->pegs[widget->nbPegsPerTurn - 1] );
			screenpos turnUL = SCREENPOS( lastPegBR.x + 3, lastPegBR.y - 1 );
			row->turn = rect_make( turnUL, VEC2U16( 7, 1 ) );
		}

		// Pins
		{
			screenpos const turnBR = rect_get_br_corner( &row->turn );
			screenpos const pinUL = SCREENPOS( turnBR.x + 3, turnBR.y - 2 );

			// first row
			u8 const firstRowLimit = ( widget->nbPegsPerTurn + 1 ) / 2;
			for ( int x = 0; x < firstRowLimit; ++x )
			{
				row->pins[x] = rect_make( SCREENPOS( pinUL.x + 5 * x, pinUL.y ), VEC2U16( 4, 2 ) );
			}
			for ( int x = firstRowLimit; x < widget->nbPegsPerTurn; ++x )
			{
				row->pins[x] = rect_make( SCREENPOS( pinUL.x + 5 * ( x - firstRowLimit ), pinUL.y + 3 ), VEC2U16( 4, 2 ) );
			}
		}
    }

	for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
	{
        usize const pegSize = ( widget->nbPegsPerTurn * 8 ) - 2;
        usize const spacesNeeded = ( TOTAL_BOARD_WIDTH - pegSize ) / 2;
        struct Rect *rect = &widget->solution[x];
	    screenpos const pos = {
		    .x = ul.x + spacesNeeded + ( 8 * x ),
			.y = ul.y + 20
		};
		*rect = rect_make( pos, VEC2U16( 6, 3 ) );
    }
}


static void draw_peg_at( struct WidgetGameBoard const *widget, usize const turn, usize const index, struct Peg const peg )
{
    if ( turn == Mastermind_SOLUTION_TURN )
    {
        screenpos const ul = rect_get_ul_corner( &widget->solution[index] );
        peg_write_6x3( ul, peg, false );
        return;
    }

    usize const turnIdx = turn - ( widget->lastDispTurn - ( ROWS_DISPLAYED - 1 ));
    screenpos const ul = rect_get_ul_corner( &widget->rows[turnIdx].pegs[index] );
    peg_write_6x3( ul, peg, false );
}


static void draw_pin_at( struct WidgetGameBoard const *widget, usize const turn, usize const index, struct Pin const pin )
{
    usize const turnIdx = turn - ( widget->lastDispTurn - ( ROWS_DISPLAYED - 1 ));
	screenpos const ul = rect_get_ul_corner( &widget->rows[turnIdx].pins[index] );

	pin_write_4x2( ul, pin );
}


static void draw_solution_at( struct WidgetGameBoard *widget, usize const index, struct Peg const peg )
{
    screenpos const solutionPos = rect_get_ul_corner( &widget->solution[index] );
    peg_write_6x3( solutionPos, peg, false );
}


static void draw_turn_at( struct WidgetGameBoard *widget, usize const rowIdx, usize const turn )
{
    if ( turn == Mastermind_SOLUTION_TURN ) return;

    screenpos const ul = rect_get_ul_corner( &widget->rows[rowIdx].turn );
    cursor_update_pos( ul );
    usize const currPlayerTurn = mastermind_get_player_turn();
    if ( currPlayerTurn == turn )
    {
        style_update( STYLE( FGColor_YELLOW ) );
    }
    else if ( currPlayerTurn > turn )
    {
        style_update( STYLE( FGColor_WHITE ) );
    }
    else
    {
        style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
    }
	term_write( L"Turn %02u", turn );
}


static void draw_turns( struct WidgetGameBoard *widget )
{
    usize const firstTurn = ( widget->lastDispTurn - ROWS_DISPLAYED ) + 1;
    for ( int y = 0; y < ROWS_DISPLAYED; ++y )
    {
        draw_turn_at( widget, y, firstTurn + y );
    }
}


static void clear_row( struct WidgetGameBoard *widget, usize const rowIdx )
{
    if ( rowIdx == Mastermind_SOLUTION_TURN )
    {
        for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
        {
            rect_clear( &widget->solution[x] );
        }
        return;
    }

 	for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
	{
        rect_clear( &widget->rows[rowIdx].pegs[x] );
    }
    rect_clear( &widget->rows[rowIdx].turn );

	for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
	{
        rect_clear( &widget->rows[rowIdx].pins[x] );
    }   
}


static void display_moved( struct WidgetGameBoard *widget, bool const historyUp )
{
    usize const nbRowsToDisplay = widget->lastDispTurn == Mastermind_SOLUTION_TURN ? 3 : 4;
    usize const firstTurn = widget->lastDispTurn == Mastermind_SOLUTION_TURN ? widget->nbTurns - 2 : ( widget->lastDispTurn - ROWS_DISPLAYED ) + 1;

    if ( historyUp && widget->lastDispTurn == widget->nbTurns )
    {
        clear_row( widget, Mastermind_SOLUTION_TURN );
    }

    for ( usize y = 0; y < nbRowsToDisplay; ++y )
	{
        usize const dispTurn = firstTurn + y;
        struct BoardRow *row = &widget->rows[y];
		for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
		{
            draw_peg_at( widget, dispTurn, x, mastermind_get_peg( dispTurn, x ) );
        }

        draw_turn_at( widget, y, dispTurn );

		for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
		{
            draw_pin_at( widget, dispTurn, x, mastermind_get_pin( dispTurn, x ) );
        }
    }

    if ( widget->lastDispTurn == Mastermind_SOLUTION_TURN )
    {
        clear_row( widget, 3 );
        struct Peg const *solution = mastermind_get_solution();
        for ( usize x = 0; x < widget->nbPegsPerTurn; ++x )
		{
            draw_peg_at( widget, widget->lastDispTurn, x, solution[x] );
        }
    }
}


static void on_trigger_confirm_turn( bool )
{
    struct Request const req = (struct Request) {
        .type = RequestType_CONFIRM_TURN
    };
    request_send( &req );
}


static void on_trigger_reset_turn( bool )
{
    struct Request const req = (struct Request) {
        .type = RequestType_RESET_TURN
    };
    request_send( &req );
}


static void on_trigger_abandon_game( bool )
{
    struct Request const req = (struct Request) {
        .type = RequestType_ABANDON_GAME
    };
    request_send( &req );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetGameBoard *widget = (struct WidgetGameBoard *)subscriber;

    switch( event->type )
    {
        case EventType_MOUSE_MOVED:
        {
            for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
            {
                //check_hovered_peg( widget, event->mouseMoved.pos );
                if ( uibutton_check_hovered( widget->buttons[idx], event->mouseMoved.pos ) )
                {
                    break;
                }
            }
            break;
        }
        case EventType_USER_INPUT:
        {
            for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
            {
                if ( uibutton_check_interaction( widget->buttons[idx], event->userInput.input ) )
                {
                    return EventPropagation_STOP;
                }
            }

            if ( event->userInput.input == keybinding_get_binded_key( KeyBinding_NEXT ) )
            {
                struct Request req = (struct Request) {
                    .type = RequestType_NEXT
                };
                request_send( &req );
            }
            else if ( event->userInput.input == keybinding_get_binded_key( KeyBinding_PREVIOUS ) )
            {
                struct Request req = (struct Request) {
                    .type = RequestType_PREVIOUS
                };
                request_send( &req );
            }
            else if ( event->userInput.input == keybinding_get_binded_key( KeyBinding_CLEAR_PEG ) )
            {
                struct Request req = (struct Request) {
                    .type = RequestType_PEG_REMOVE
                };
                request_send( &req );
            }
            else if ( event->userInput.input == keybinding_get_binded_key( KeyBinding_HISTORY_UP ) )
            {
                if ( widget->lastDispTurn > 4 )
                {
                    widget->lastDispTurn -= 1;
                    display_moved( widget, true );
                }
                else if ( widget->lastDispTurn == Mastermind_SOLUTION_TURN )
                {
                    widget->lastDispTurn = widget->nbTurns;
                    display_moved( widget, true );
                }
            }
            else if ( event->userInput.input == keybinding_get_binded_key( KeyBinding_HISTORY_DOWN ) )
            {
                if ( widget->lastDispTurn != Mastermind_SOLUTION_TURN )
                {
                    widget->lastDispTurn += 1;
                    if ( widget->lastDispTurn > widget->nbTurns ) widget->lastDispTurn = Mastermind_SOLUTION_TURN;
                    display_moved( widget, false );
                }
            }
            break;
        }
        case EventType_GAME_LOST:
        case EventType_GAME_WON:
        {
            if ( widget->lastDispTurn != Mastermind_SOLUTION_TURN )
            {
                widget->lastDispTurn = Mastermind_SOLUTION_TURN;
                display_moved( widget, false );
            }
            for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
            {
                uibutton_hide( widget->buttons[idx] );
            }            
            break;
        }
        case EventType_GAME_NEW:
        {
            if ( widget->nbPegsPerTurn != event->newGame.nbPegsPerTurn || widget->nbTurns != event->newGame.nbTurns )
            {
                rect_clear_content( &widget->box );
                draw_internal_board_lines( widget );
                widget->nbPegsPerTurn = event->newGame.nbPegsPerTurn;
                widget->nbTurns = event->newGame.nbTurns;
            }
            init_widget_data( widget );
            draw_turns( widget );
            for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
            {
                uibutton_show( widget->buttons[idx] );
            }
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
                if ( widget->lastDispTurn == Mastermind_SOLUTION_TURN )
                {
                    draw_solution_at( widget, evPeg->index, evPeg->peg );
                }
            }
            else
            {
                if ( evPeg->turn <= widget->lastDispTurn && evPeg->turn >= widget->lastDispTurn - ( ROWS_DISPLAYED - 1 ) )
                {
                    draw_peg_at( widget, evPeg->turn, evPeg->index, evPeg->peg );
                }
            }
            break;
        }

        case EventType_PIN_ADDED:
        case EventType_PIN_REMOVED:
        {
            struct EventPin const *evPin = &event->pin;
            if ( evPin->turn <= widget->lastDispTurn && evPin->turn >= widget->lastDispTurn - ( ROWS_DISPLAYED - 1 ) )
            {
                draw_pin_at( widget, evPin->turn, evPin->index, evPin->pin );
            }
            break;
        }
    }
    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
	struct WidgetGameBoard *widget = (struct WidgetGameBoard *)base;
	rect_draw_borders( &widget->box, L"Mastermind Board" );
	draw_internal_board_lines( widget );
}


static void disable_callback( struct Widget *base )
{
    struct WidgetGameBoard *widget = (struct WidgetGameBoard *)base;
	for ( usize idx = 0; idx < ButtonIdx_Count; ++idx )
	{
		uibutton_hide( widget->buttons[idx] );
	}
	rect_clear( &widget->box );    
}


struct Widget *widget_game_board_create( void )
{
    struct WidgetGameBoard *const widget = calloc( 1, sizeof( struct WidgetGameBoard ) );
    if ( !widget ) return NULL;

    widget->base.name = "GameBoard";
    widget->base.enabledScenes = UIScene_IN_GAME;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    // Widget specific

  	widget->box = rect_make( SCREENPOS( 17, 2 ), VEC2U16( TOTAL_BOARD_WIDTH, 27 ) );

	screenpos const bul = SCREENPOS( 18, 27 );
    widget->buttons[ButtonIdx_CONFIRM_TURN] = uibutton_register( L"Confirm Turn", SCREENPOS( bul.x + 3, bul.y ), VEC2U16( 19, 1 ), Keybinding_CONFIRM_TURN, on_trigger_confirm_turn, true );
    widget->buttons[ButtonIdx_RESET_TURN]   = uibutton_register( L"Reset Turn", SCREENPOS( bul.x + 25, bul.y ), VEC2U16( 13, 1 ), Keybinding_RESET_TURN, on_trigger_reset_turn, true );
    widget->buttons[ButtonIdx_ABANDON_GAME] = uibutton_register( L"Abandon Game", SCREENPOS( bul.x + 58, bul.y ), VEC2U16( 15, 1 ), Keybinding_ABANDON_GAME, on_trigger_abandon_game, true );

    event_register( widget, on_event_callback );
    event_subscribe( widget, EventType_MaskAll );

    return (struct Widget *)widget;
}