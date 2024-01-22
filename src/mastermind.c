#include "mastermind.h"

#include "characters_list.h"
#include "keyboard_inputs.h"
#include "settings.h"
#include "components/components.h"
#include "gameloop.h"

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
static MastermindCallback s_callbacks[Mastermind_MAX_CALLBACKS] = {};
static u8 s_callbackCount = 0;


static void emit_game_update( enum GameUpdateType const type )
{
    for ( int idx = 0; idx < s_callbackCount; ++idx )
    {
        s_callbacks[idx]( type );
    }
}

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
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        enum PieceTurn const turnStatus = piece_turn_status( turn );
        pegs[idx] = Piece_TypePeg | PieceFlag_EMPTY | turnStatus;
    }
}

static void reset_pins_row( byte *const pins, usize const turn )
{
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        enum PieceTurn const turnStatus = piece_turn_status( turn );
        pins[idx] = Piece_TypePin | PieceFlag_EMPTY | turnStatus;
    }
}

static void generate_new_solution( byte *const pegs )
{
    // Note: For the moment, we aren't supporting having duplicated pegs in the same row.
    // So this generation is explicitly checking to have unique pegs in the generated solution.
    bool pegsUsed[Mastermind_NB_COLORS] = {};

    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
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
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        s_mastermind.solution[idx] |= PieceFlag_SECRET;
    }
}


static void show_solution( void )
{
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        s_mastermind.solution[idx] &= ~PieceFlag_SECRET;
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

    while ( nbCorrect-- > 0 ) { pinsTurn[pinIdx++] = Piece_PIN_CORRECT; }
    while ( nbPartial-- > 0 ) { pinsTurn[pinIdx++] = Piece_PIN_INCORRECT; }
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


static bool abandon_game( void )
{
    if ( s_mastermind.gameStatus == GameStatus_IN_PROGRESS )
    {
        s_mastermind.gameStatus = GameStatus_LOST;
        show_solution();
        gameloop_emit_event( EventType_GAME_LOST, NULL );
        return true;
    }
    return false;
}


static bool new_game( u8 const nbTurns, u8 const nbPiecesPerTurn, enum GameExperience const gameExperience )
{
    // Settings
    s_mastermind.nbTurns = nbTurns;
    s_mastermind.nbPiecesPerTurn = nbPiecesPerTurn;
    s_mastermind.gameExperience = gameExperience;

    // Game logic
    s_mastermind.currentTurn = 1;
    s_mastermind.selectionBarIdx = 0;
    s_mastermind.gameStatus = GameStatus_IN_PROGRESS;

    // Game data
    for ( int idx = 0; idx < Mastermind_MAX_TURNS; ++idx )
    {
        reset_pegs_row( s_mastermind.pegs[idx], idx + 1 );
        reset_pins_row( s_mastermind.pins[idx], idx + 1 );
    }
    generate_new_solution( s_mastermind.solution );
    hide_solution();

    component_enable( ComponentId_SUMMARY );
    component_enable( ComponentId_TIMER );
    component_enable( ComponentId_PEG_SELECTION );
    component_enable( ComponentId_BOARD );

    gameloop_emit_event( EventType_NEW_GAME, NULL );
    return true;
}


bool mastermind_try_consume_input( enum KeyInput const input )
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
            emit_game_update( GameUpdateType_TURN_RESET );
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
                emit_game_update( GameUpdateType_SELECTION_BAR_MOVED );
                // EventType_HOVERED_PEG ?
            }
            return true;
        }
        case KeyInput_ARROW_RIGHT:
        {
            if ( s_mastermind.selectionBarIdx + 1 < s_mastermind.nbPiecesPerTurn )
            {
                s_mastermind.selectionBarIdx += 1;
                emit_game_update( GameUpdateType_SELECTION_BAR_MOVED );
            }
            return true;
        }

/*        case KeyInput_NUMPAD_0 ... KeyInput_NUMPAD_8:
        {
            gamepiece const piece = ( input - KeyInput_NUMPAD_0 ) | Piece_TypePeg | PieceTurn_CURRENT;
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx] = piece;
            gameloop_emit_event( EventType_PEG_ADDED );
            return true;
        }*/
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
}


bool mastermind_register_update_callback( MastermindCallback const callback )
{
    if ( callback == NULL )                            return false;
    if ( s_callbackCount == Mastermind_MAX_CALLBACKS ) return false;

    s_callbacks[s_callbackCount] = callback;
    s_callbackCount++;
    return true;    
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

u8 mastermind_get_selection_bar_index( struct Mastermind const *mastermind )
{
    return mastermind->selectionBarIdx;
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

struct Mastermind const *mastermind_get_instance( void )
{
    return &s_mastermind;
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
