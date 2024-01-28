#include "mastermind.h"

#include "characters_list.h"
#include "keyboard_inputs.h"
#include "settings.h"
#include "components/components.h"
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
    gamepiece pegs[Mastermind_MAX_TURNS][Mastermind_MAX_PIECES_PER_TURN];
    gamepiece pins[Mastermind_MAX_TURNS][Mastermind_MAX_PIECES_PER_TURN];
    gamepiece solution[Mastermind_MAX_PIECES_PER_TURN];

    // Game logic
    u8 currentTurn;
    u8 selectionBarIdx;
    enum GameStatus gameStatus;
};


static struct Mastermind s_mastermind = {};


static enum PieceTurn piece_turn_status( usize turn )
{
    if ( turn == s_mastermind.currentTurn )
    {
        return PieceTurn_CURRENT;
    }
    else if ( turn > s_mastermind.currentTurn )
    {
        return PieceTurn_FUTURE;
    }
    else
    {
        return PieceTurn_PAST;
    }
}


static void reset_pegs_row( byte *const pegs, usize const turn )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
//        enum PieceTurn const turnStatus = piece_turn_status( turn );
        pegs[idx] = Piece_TypePeg | PieceFlag_EMPTY;// | turnStatus;

        struct Event const event = EVENT_PEG( EventType_PEG_REMOVED, turn, idx, pegs[idx] );
        event_trigger( &event );
    }
}


static void reset_pins_row( byte *const pins, usize const turn )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
//        enum PieceTurn const turnStatus = piece_turn_status( turn );
        pins[idx] = Piece_TypePin | Piece_PIN_INCORRECT;// | turnStatus;

        struct Event const event = EVENT_PIN( EventType_PIN_REMOVED, turn, idx, pins[idx] );
        event_trigger( &event );
    }
}


static void generate_new_solution( byte *const pegs )
{
    // Note: For the moment, we aren't supporting having duplicated pegs in the same row.
    // So this generation is explicitly checking to have unique pegs in the generated solution.
    bool pegsUsed[Mastermind_NB_COLORS] = {};

    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        do
		{
            pegs[idx] = rand() % Mastermind_NB_COLORS;
		} while ( pegsUsed[pegs[idx]] );

		pegsUsed[pegs[idx]] = true;
    }
}


static void hide_solution( void )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( ( s_mastermind.solution[idx] & PieceFlag_SECRET ) == 0 )
        {
            s_mastermind.solution[idx] |= PieceFlag_SECRET;
            struct Event const event = EVENT_PEG( EventType_PEG_HIDDEN, Mastermind_SOLUTION_TURN, idx, s_mastermind.solution[idx] );
            event_trigger( &event );
        }
    }
}


static void reveal_solution( void )
{
    for ( int idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( s_mastermind.solution[idx] & PieceFlag_SECRET )
        {
            s_mastermind.solution[idx] &= ~PieceFlag_SECRET;
            struct Event const event = EVENT_PEG( EventType_PEG_REVEALED, Mastermind_SOLUTION_TURN, idx, s_mastermind.solution[idx] );
            event_trigger( &event );
        }
    }
}


static bool is_current_turn_valid( void )
{
    bool alreadyUsed[Mastermind_NB_COLORS];
    memset( alreadyUsed, 0, sizeof( alreadyUsed ) );

    gamepiece const *pegsTurn = mastermind_get_pegs_at_turn( s_mastermind.currentTurn );
    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        gamepiece const piece = pegsTurn[idx];
        if ( ( piece & PieceFlag_EMPTY ) || alreadyUsed[piece & Piece_MaskColor] )
        {
            return false;
        }

        alreadyUsed[piece & Piece_MaskColor] = true;
    }

    return true;
}


static void generate_feedback_on_current_turn( void )
{
    gamepiece const *pegsTurn = mastermind_get_pegs_at_turn( s_mastermind.currentTurn );
    gamepiece const *solution = mastermind_get_solution();

    bool alreadyUsed[s_mastermind.nbPiecesPerTurn];
    memset( alreadyUsed, 0, sizeof( alreadyUsed ) );

    // First step, detect the number of CORRECT/PARTIALLY_CORRECT
    int nbCorrect = 0;
    int nbPartial = 0;

    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( !alreadyUsed[idx] && ( solution[idx] & Piece_MaskColor ) == ( pegsTurn[idx] & Piece_MaskColor ) )
        {
            alreadyUsed[idx] = true;
            nbCorrect += 1;
            continue;
        }

        for ( usize partialIdx = 0; partialIdx < s_mastermind.nbPiecesPerTurn; ++partialIdx )
        {
            if ( partialIdx == idx || alreadyUsed[partialIdx] ) continue;
            if ( ( solution[idx] & Piece_MaskColor ) == ( pegsTurn[partialIdx] & Piece_MaskColor ) )
            {
                alreadyUsed[partialIdx] = true;
                nbPartial += 1;
                break;
            }
        }
    }

    // Next step, fill the feedback with the corresponding pins
    gamepiece *pinsTurn = s_mastermind.pins[s_mastermind.currentTurn - 1];
    usize pinIdx = 0;

    while ( nbCorrect-- > 0 ){ pinsTurn[pinIdx++] = Piece_PIN_CORRECT; }
    while ( nbPartial-- > 0 ) { pinsTurn[pinIdx++] = Piece_PIN_PARTIAL; }

    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; idx++ )
    {
        struct Event const event = EVENT_PIN( EventType_PIN_ADDED, s_mastermind.currentTurn, idx, pinsTurn[idx] );
        event_trigger( &event );
    }
}


static bool is_current_turn_match_solution( void )
{
    gamepiece const *pins = mastermind_get_pins_at_turn( s_mastermind.currentTurn );
    for ( usize idx = 0; idx < s_mastermind.nbPiecesPerTurn; ++idx )
    {
        if ( ( pins[idx] & Piece_MaskAll ) != Piece_PIN_CORRECT )
        {
            return false;
        }
    }

    return true;   
}


static void next_turn( void )
{
    gamepiece *currPeg = s_mastermind.pegs[s_mastermind.currentTurn - 1];
    gamepiece *nextPeg = s_mastermind.pegs[s_mastermind.currentTurn];

    gamepiece *currPin = s_mastermind.pins[s_mastermind.currentTurn - 1];
    gamepiece *nextPin = s_mastermind.pins[s_mastermind.currentTurn];

    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        currPeg[idx] = ( currPeg[idx] & ~PieceTurn_MaskAll ) | PieceTurn_PAST;
        nextPeg[idx] = ( nextPeg[idx] & ~PieceTurn_MaskAll ) | PieceTurn_CURRENT;

        currPin[idx] = ( currPin[idx] & ~PieceTurn_MaskAll ) | PieceTurn_PAST;
        nextPin[idx] = ( nextPin[idx] & ~PieceTurn_MaskAll ) | PieceTurn_CURRENT;
    }

    s_mastermind.currentTurn += 1;
    s_mastermind.selectionBarIdx = 0;   
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

    gamepiece piece = s_mastermind.pegs[turn - 1][idx];
    if ( piece & PieceFlag_EMPTY ) return RequestStatus_SKIPPED;

    piece |= PieceFlag_EMPTY;

    s_mastermind.pegs[turn - 1][idx] = piece;

    struct Event const event = EVENT_PEG( EventType_PEG_REMOVED, turn, idx, piece );
    event_trigger( &event );

    return RequestStatus_TREATED;
}


static enum RequestStatus on_request_add_peg( gamepiece const pieceColor )
{
    if ( mastermind_is_game_finished() ) return RequestStatus_SKIPPED;

    gamepiece piece = s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx];
    if ( !( piece & PieceFlag_EMPTY ) )
    {
        on_request_remove_peg( s_mastermind.currentTurn, s_mastermind.selectionBarIdx );
    }

    piece &= ~( Piece_MaskColor | PieceFlag_EMPTY );
    piece |= pieceColor;

    s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx] = piece;

    struct Event const event = EVENT_PEG( EventType_PEG_ADDED, s_mastermind.currentTurn, s_mastermind.selectionBarIdx, piece );
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

/*bool mastermind_try_consume_input( enum KeyInput const input )
{
    switch( input )
    {
        case KeyInput_SPACE:
        {
            u8 const nbTurns = settings_get_nb_turns();
            u8 const nbPiecesPerTurn = settings_get_nb_pieces_per_turn();
            enum GameExperience gameExperience = settings_get_game_experience();
            new_game( nbTurns, nbPiecesPerTurn, gameExperience );
            return true;
        }
        case KeyInput_Q:
        {
            abandon_game();
            return true;
        }
        case KeyInput_R:
        {
            reset_pegs_row( s_mastermind.pegs[s_mastermind.currentTurn - 1], s_mastermind.currentTurn );
            //emit_game_update( GameUpdateType_TURN_RESET );
            return true;
        }
        case KeyInput_ENTER:
        {
            if ( !is_current_turn_valid() ) return true;

            generate_feedback_on_current_turn();

            if ( is_current_turn_match_solution() )
            {
                s_mastermind.gameStatus = GameStatus_WON;
                show_solution();
                gameloop_emit_event( EventType_GAME_WON, NULL );
            }
            else if ( s_mastermind.currentTurn == s_mastermind.nbTurns )
            {
                s_mastermind.gameStatus = GameStatus_LOST;
                show_solution();
                gameloop_emit_event( EventType_GAME_LOST, NULL );
            }
            else
            {
                next_turn();
                gameloop_emit_event( EventType_NEXT_TURN, NULL );
            }
            return true;
        }
        case KeyInput_ARROW_LEFT:
        {
            if ( s_mastermind.selectionBarIdx > 0 )
            {
                s_mastermind.selectionBarIdx -= 1;
                //emit_game_update( GameUpdateType_SELECTION_BAR_MOVED );
                // EventType_HOVERED_PEG ?
            }
            return true;
        }
        case KeyInput_ARROW_RIGHT:
        {
            if ( s_mastermind.selectionBarIdx + 1 < s_mastermind.nbPiecesPerTurn )
            {
                s_mastermind.selectionBarIdx += 1;
                //emit_game_update( GameUpdateType_SELECTION_BAR_MOVED );
            }
            return true;
        }

        case KeyInput_NUMPAD_0 ... KeyInput_NUMPAD_8:
        {
            gamepiece const piece = ( input - KeyInput_NUMPAD_0 ) | Piece_TypePeg | PieceTurn_CURRENT;
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx] = piece;
            gameloop_emit_event( EventType_PEG_ADDED );
            return true;
        }
        case KeyInput_0 ... KeyInput_7:
        {
            // check first if the peg added is the same or not as before.
            gamepiece const piece = ( input - KeyInput_0 ) | Piece_TypePeg | PieceTurn_CURRENT;
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx] = piece;
            gameloop_emit_event( EventType_PEG_ADDED, NULL );
            return true;
        }
        case KeyInput_BACKSPACE:
        {
            // Check first if the peg added is already empty
            enum PieceTurn const turnStatus = piece_turn_status( s_mastermind.currentTurn );
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx] = Piece_TypePeg | PieceFlag_EMPTY | turnStatus;
            gameloop_emit_event( EventType_PEG_REMOVED, NULL );
            return true;
        }
    }
    return false;
}*/


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


gamepiece mastermind_get_peg( usize const turn, usize const index )
{
    return s_mastermind.pegs[turn - 1][index];
}

gamepiece mastermind_get_pin( usize const turn, usize const index )
{
    return s_mastermind.pins[turn - 1][index];
}

gamepiece const *mastermind_get_pegs_at_turn( usize const turn )
{
    assert( turn > 0 && turn <= s_mastermind.nbTurns );
    return s_mastermind.pegs[turn - 1];
}

gamepiece const *mastermind_get_pins_at_turn( usize const turn )
{
    assert( turn > 0 && turn <= s_mastermind.nbTurns );
    return s_mastermind.pins[turn - 1];
}

gamepiece const *mastermind_get_solution( void )
{
    return s_mastermind.solution;
}


enum RequestStatus mastermind_on_request( struct Request const *req )
{
    switch ( req->type )
    {
        case RequestType_START_NEW_GAME: return on_request_start_new_game();
        case RequestType_ABANDON_GAME:   return on_request_abandon_game();

        case RequestType_PEG_SELECT:
        { /* TODO */ }
        case RequestType_PEG_UNSELECT:
        { /* TODO */ }

        case RequestType_PEG_ADD: return on_request_add_peg( req->pegAdd.piece );
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
    }

    return RequestStatus_SKIPPED;
}