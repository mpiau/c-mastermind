#include "mastermind.h"

#include "characters_list.h"
#include "keyboard_inputs.h"
#include "settings.h"
#include "events.h"
#include "gameloop.h"
#include "ui/ui.h"

#include <stdlib.h>
#include <string.h>


struct Mastermind
{
    // Game settings. Can't be changed without creating a new game
    u8 nbTurns;
    u8 nbPiecesPerTurn;
    enum GameExperience gameExperience;

    // Game data
    struct Peg pegs[Mastermind_MAX_TURNS][Mastermind_MAX_PIECES_PER_TURN];
    struct Pin pins[Mastermind_MAX_TURNS][Mastermind_MAX_PIECES_PER_TURN];
    struct Peg solution[Mastermind_MAX_PIECES_PER_TURN];

    // Game logic
    u8 currentTurn;
    u8 selectionBarIdx;
    enum GameStatus gameStatus;
    enum PegId selected;
};


static struct Mastermind s_mastermind = {};


static void reset_pegs_row( struct Peg *const pegs, usize const turn )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        pegs[idx].id = PegId_EMPTY;
        pegs[idx].hidden = false;

        struct Event const event = EVENT_PEG( EventType_PEG_REMOVED, turn, idx, pegs[idx] );
        event_trigger( &event );
    }
}


static void reset_pins_row( struct Pin *const pins, usize const turn )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        pins[idx].id = PinId_INCORRECT;

        struct Event const event = EVENT_PIN( EventType_PIN_REMOVED, turn, idx, pins[idx] );
        event_trigger( &event );
    }
}


static void generate_new_solution( struct Peg *const pegs )
{
    // Note: For the moment, we aren't supporting having duplicated pegs in the same row.
    // So this generation is explicitly checking to have unique pegs in the generated solution.
    bool pegsUsed[Mastermind_NB_COLORS] = {};

    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        do
		{
            pegs[idx].id = rand() % Mastermind_NB_COLORS;
		} while ( pegsUsed[pegs[idx].id] );

		pegsUsed[pegs[idx].id] = true;
    }
}


static void hide_solution( void )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( !s_mastermind.solution[idx].hidden )
        {
            s_mastermind.solution[idx].hidden = true;
            struct Event const event = EVENT_PEG( EventType_PEG_HIDDEN, Mastermind_SOLUTION_TURN, idx, s_mastermind.solution[idx] );
            event_trigger( &event );
        }
    }
}


static void reveal_solution( void )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( s_mastermind.solution[idx].hidden )
        {
            s_mastermind.solution[idx].hidden = false;
            struct Event const event = EVENT_PEG( EventType_PEG_REVEALED, Mastermind_SOLUTION_TURN, idx, s_mastermind.solution[idx] );
            event_trigger( &event );
        }
    }
}


static bool is_current_turn_valid( void )
{
    bool alreadyUsed[Mastermind_NB_COLORS];
    memset( alreadyUsed, 0, sizeof( alreadyUsed ) );

    struct Peg const *pegsTurn = mastermind_get_pegs_at_turn( s_mastermind.currentTurn );
    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        struct Peg const peg = pegsTurn[idx];
        if ( peg.id == PegId_EMPTY || alreadyUsed[peg.id] )
        {
            return false;
        }

        assert( peg.id != PegId_EMPTY );
        alreadyUsed[peg.id] = true;
    }

    return true;
}


static void generate_feedback_on_current_turn( void )
{
    struct Peg const *pegsTurn = mastermind_get_pegs_at_turn( s_mastermind.currentTurn );
    struct Peg const *solution = mastermind_get_solution();

    bool alreadyUsed[s_mastermind.nbPiecesPerTurn];
    memset( alreadyUsed, 0, sizeof( alreadyUsed ) );

    // First step, detect the number of CORRECT/PARTIALLY_CORRECT
    int nbCorrect = 0;
    int nbPartial = 0;

    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( !alreadyUsed[idx] && solution[idx].id == pegsTurn[idx].id )
        {
            alreadyUsed[idx] = true;
            nbCorrect += 1;
            continue;
        }

        for ( usize partialIdx = 0; partialIdx < s_mastermind.nbPiecesPerTurn; ++partialIdx )
        {
            if ( partialIdx == idx || alreadyUsed[partialIdx] ) continue;
            if ( solution[idx].id == pegsTurn[partialIdx].id )
            {
                alreadyUsed[partialIdx] = true;
                nbPartial += 1;
                break;
            }
        }
    }

    // Next step, fill the feedback with the corresponding pins
    struct Pin *pinsTurn = s_mastermind.pins[s_mastermind.currentTurn - 1];
    usize pinIdx = 0;

    while ( nbCorrect-- > 0 ){ pinsTurn[pinIdx++].id = PinId_CORRECT; }
    while ( nbPartial-- > 0 ) { pinsTurn[pinIdx++].id = PinId_PARTIAL; }

    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; idx++ )
    {
        struct Event const event = EVENT_PIN( EventType_PIN_ADDED, s_mastermind.currentTurn, idx, pinsTurn[idx] );
        event_trigger( &event );
    }
}


static bool is_current_turn_match_solution( void )
{
    struct Pin const *pins = mastermind_get_pins_at_turn( s_mastermind.currentTurn );
    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( pins[idx].id != PinId_CORRECT )
        {
            return false;
        }
    }

    return true;   
}


static enum RequestStatus on_request_abandon_game( void )
{
    if ( s_mastermind.gameStatus == GameStatus_IN_PROGRESS )
    {
        reveal_solution();
        s_mastermind.gameStatus = GameStatus_LOST;
        // Emit a show solution event
        struct Event const event = (struct Event) {
            .type = EventType_GAME_LOST
        };
        event_trigger( &event );
        return RequestStatus_TREATED;
    }
    return RequestStatus_SKIPPED;
}


static enum RequestStatus on_request_start_new_game( void )
{
    u8 const nbTurns = settings_get_nb_turns();
    u8 const nbPiecesPerTurn = settings_get_nb_pieces_per_turn();
    enum GameExperience gameExperience = settings_get_game_experience();

    // Settings
    s_mastermind.nbTurns = nbTurns;
    s_mastermind.nbPiecesPerTurn = nbPiecesPerTurn;
    s_mastermind.gameExperience = gameExperience;

    // Game logic
    s_mastermind.currentTurn = 1;
    s_mastermind.selected = PegId_EMPTY;
    s_mastermind.selectionBarIdx = 0;
    s_mastermind.gameStatus = GameStatus_IN_PROGRESS;

    ui_change_scene( UIScene_IN_GAME );

    struct Event event = EVENT_GAME_NEW( s_mastermind.nbTurns, s_mastermind.nbPiecesPerTurn );
    event_trigger( &event );

    // Game data
    for ( int idx = 0; idx < Mastermind_MAX_TURNS; ++idx )
    {
        reset_pegs_row( s_mastermind.pegs[idx], idx + 1 );
        reset_pins_row( s_mastermind.pins[idx], idx + 1 );
    }

    generate_new_solution( s_mastermind.solution );
    hide_solution();

    event = (struct Event) {
        .type = EventType_NEW_TURN,
        .newTurn = (struct EventNewTurn) {
            .turn = s_mastermind.currentTurn
        }
    };
    event_trigger( &event );

    return RequestStatus_TREATED;
}


static enum RequestStatus on_request_remove_peg( usize const turn, usize const idx )
{
    if ( mastermind_is_game_finished() ) return RequestStatus_SKIPPED;

    struct Peg peg = s_mastermind.pegs[turn - 1][idx];
    if ( peg.id == PegId_EMPTY ) return RequestStatus_SKIPPED;

    peg.id = PegId_EMPTY;

    s_mastermind.pegs[turn - 1][idx] = peg;

    struct Event const event = EVENT_PEG( EventType_PEG_REMOVED, turn, idx, peg );
    event_trigger( &event );

    return RequestStatus_TREATED;
}


static enum RequestStatus on_request_add_peg( enum PegId const id )
{
    if ( mastermind_is_game_finished() ) return RequestStatus_SKIPPED;

    struct Peg peg = s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx];
    // The peg is already on the position.
    if ( peg.id == id ) return RequestStatus_SKIPPED;

    if ( !settings_is_duplicate_allowed() )
    {
        for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
        {
            if ( idx == s_mastermind.selectionBarIdx ) continue;

            struct Peg pieceOnBoard = s_mastermind.pegs[s_mastermind.currentTurn - 1][idx];
            if ( pieceOnBoard.id == id )
            {
                on_request_remove_peg( s_mastermind.currentTurn, idx );
                break;
            }
        }
    }

    if ( peg.id != PegId_EMPTY )
    {
        on_request_remove_peg( s_mastermind.currentTurn, s_mastermind.selectionBarIdx );
    }

    peg.id = id;

    s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx] = peg;

    struct Event const event = EVENT_PEG( EventType_PEG_ADDED, s_mastermind.currentTurn, s_mastermind.selectionBarIdx, peg );
    event_trigger( &event );

    return RequestStatus_TREATED;
}


static enum RequestStatus on_request_select_peg( enum PegId const id )
{
    if ( s_mastermind.selected == id ) return RequestStatus_SKIPPED;

    s_mastermind.selected = id;
    struct Peg peg = (struct Peg) { .hidden = false, .id = id };

    struct Event const event = EVENT_PEG( EventType_PEG_SELECTED, s_mastermind.currentTurn, s_mastermind.selectionBarIdx, peg );
    event_trigger( &event );

    return RequestStatus_TREATED;
}


static enum RequestStatus on_request_unselect_peg( enum PegId const id )
{
    if ( s_mastermind.selected == PegId_EMPTY ) return RequestStatus_SKIPPED;

    s_mastermind.selected = id;
    struct Peg peg = (struct Peg) { .hidden = false, .id = id };

    struct Event const event = EVENT_PEG( EventType_PEG_UNSELECTED, s_mastermind.currentTurn, s_mastermind.selectionBarIdx, peg );
    event_trigger( &event );

    return RequestStatus_TREATED;
}


static enum RequestStatus on_request_reset_turn( void )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        on_request_remove_peg( s_mastermind.currentTurn, idx );
    }
    return RequestStatus_TREATED;
}


static enum RequestStatus on_request_confirm_turn( void )
{
    if ( !is_current_turn_valid() ) return RequestStatus_SKIPPED;

    generate_feedback_on_current_turn();

    if ( is_current_turn_match_solution() )
    {
        reveal_solution();
        s_mastermind.gameStatus = GameStatus_WON;
        struct Event event = (struct Event) { .type = EventType_GAME_WON };
        event_trigger( &event );
    }
    else if ( s_mastermind.currentTurn == s_mastermind.nbTurns )
    {
        s_mastermind.gameStatus = GameStatus_LOST;
        reveal_solution();
        struct Event event = (struct Event) { .type = EventType_GAME_LOST };
        event_trigger( &event );
    }
    else
    {
        s_mastermind.currentTurn += 1;
        s_mastermind.selectionBarIdx = 0;  

        struct Event event = (struct Event) {
            .type = EventType_NEW_TURN,
            .newTurn = (struct EventNewTurn) {
                .turn = s_mastermind.currentTurn
            }
        };
        event_trigger( &event );
    }

    return RequestStatus_TREATED;
}


usize mastermind_get_total_turns( void )
{
    return s_mastermind.nbTurns;
}

usize mastermind_get_nb_pieces_per_turn( void )
{
    return s_mastermind.nbPiecesPerTurn;
}

usize mastermind_get_player_turn( void )
{
    return s_mastermind.currentTurn;
}

u8 mastermind_get_selection_bar_index( void )
{
    return s_mastermind.selectionBarIdx;
}

bool mastermind_is_game_finished( void )
{
    return mastermind_is_game_lost() || mastermind_is_game_won();
}

bool mastermind_is_game_lost( void )
{
    return s_mastermind.gameStatus == GameStatus_LOST;
}

bool mastermind_is_game_won( void )
{
    return s_mastermind.gameStatus == GameStatus_WON;
}


struct Peg mastermind_get_peg( usize const turn, usize const index )
{
    return s_mastermind.pegs[turn - 1][index];
}

struct Pin mastermind_get_pin( usize const turn, usize const index )
{
    return s_mastermind.pins[turn - 1][index];
}

struct Peg const *mastermind_get_pegs_at_turn( usize const turn )
{
    assert( turn > 0 && turn <= s_mastermind.nbTurns );
    return s_mastermind.pegs[turn - 1];
}

struct Pin const *mastermind_get_pins_at_turn( usize const turn )
{
    assert( turn > 0 && turn <= s_mastermind.nbTurns );
    return s_mastermind.pins[turn - 1];
}

struct Peg const *mastermind_get_solution( void )
{
    return s_mastermind.solution;
}


enum RequestStatus mastermind_on_request( struct Request const *req )
{
    switch ( req->type )
    {
        case RequestType_START_NEW_GAME: return on_request_start_new_game();
        case RequestType_ABANDON_GAME:   return on_request_abandon_game();

        case RequestType_PEG_SELECT: return on_request_select_peg( req->peg.id );
        case RequestType_PEG_UNSELECT: return on_request_unselect_peg( req->peg.id );

        case RequestType_PEG_ADD: return on_request_add_peg( req->peg.id );
        case RequestType_PEG_REMOVE: return on_request_remove_peg( s_mastermind.currentTurn, s_mastermind.selectionBarIdx );

        case RequestType_RESET_TURN: return on_request_reset_turn();
        case RequestType_CONFIRM_TURN: return on_request_confirm_turn();

        case RequestType_NEXT:
        {
            if ( s_mastermind.selectionBarIdx + 1 < s_mastermind.nbPiecesPerTurn )
            {
                s_mastermind.selectionBarIdx += 1;
                // TODO Event
                return RequestStatus_TREATED;
            }
            break;
        }
        case RequestType_PREVIOUS:
        {
            if ( s_mastermind.selectionBarIdx > 0 )
            {
                s_mastermind.selectionBarIdx -= 1;
                // TODO Event
                return RequestStatus_TREATED;
            }
            break;
        }

        default: break;
    }

    return RequestStatus_SKIPPED;
}