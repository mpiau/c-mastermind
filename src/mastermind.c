#include "mastermind.h"

#include "characters_list.h"
#include "keyboard_inputs.h"
#include "settings.h"

struct Mastermind
{
    // Game settings. Can't be changed without creating a new game
    u8 nbTurns;
    u8 nbPegsPerTurn;
    enum GameExperience gameExperience;

    // Game data
    struct Peg pegs[Mastermind_MAX_TURNS][Mastermind_MAX_PIECES_PER_TURN];
    struct Pin pins[Mastermind_MAX_TURNS][Mastermind_MAX_PIECES_PER_TURN];
    struct Peg solution[Mastermind_MAX_PIECES_PER_TURN];

    // Game logic
    u8 currentTurn;
    u8 selectionBarIdx;
    enum PegId selectedPeg;
    enum GameStatus gameStatus;
};


static struct Mastermind s_mastermind = {};
static MastermindCallback s_callbacks[Mastermind_MAX_CALLBACKS] = {};
static u8 s_callbackCount = 0;


static void emit_game_update( enum GameUpdateType const type )
{
    for ( int idx = 0; idx < s_callbackCount; ++idx )
    {
        s_callbacks[idx]( &s_mastermind, type );
    }
}

static void reset_pegs_row( struct Peg *const pegs )
{
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        pegs[idx].id = PegId_Empty;
        pegs[idx].hidden = false;
    }
}

static void reset_pins_row( struct Pin *const pins )
{
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        pins[idx].id = PinId_INCORRECT;
    }
}

static void generate_new_solution( struct Peg *const pegs )
{
    // Note: For the moment, we aren't supporting having duplicated pegs in the same row.
    // So this generation is explicitly checking to have unique pegs in the generated solution.
    bool pegsUsed[Mastermind_NB_COLORS] = {};

    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
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
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        s_mastermind.solution[idx].hidden = true;
    }
}


static void show_solution( void )
{
    for ( int idx = 0; idx < Mastermind_MAX_PIECES_PER_TURN; ++idx )
    {
        s_mastermind.solution[idx].hidden = false;
    }
}


static bool is_current_turn_valid( void )
{
    bool alreadyUsed[Mastermind_NB_COLORS];
    memset( alreadyUsed, 0, sizeof( alreadyUsed ) );

    struct Peg const *pegsTurn = mastermind_get_pegs_at_turn( s_mastermind.currentTurn );
    for ( usize idx = 0; idx < s_mastermind.nbPegsPerTurn; ++idx )
    {
        enum PegId const id = pegsTurn[idx].id;
        if ( id == PegId_Empty || alreadyUsed[id] )
        {
            return false;
        }

        alreadyUsed[id] = true;
    }

    return true;
}


static void generate_feedback_on_current_turn( void )
{
    struct Peg const *pegsTurn = mastermind_get_pegs_at_turn( s_mastermind.currentTurn );
    struct Peg const *solution = mastermind_get_solution();

    bool alreadyUsed[s_mastermind.nbPegsPerTurn];
    memset( alreadyUsed, 0, sizeof( alreadyUsed ) );

    // First step, detect the number of CORRECT/PARTIALLY_CORRECT
    int nbCorrect = 0;
    int nbPartial = 0;

    for ( usize idx = 0; idx < s_mastermind.nbPegsPerTurn; ++idx )
    {
        if ( !alreadyUsed[idx] && solution[idx].id == pegsTurn[idx].id )
        {
            alreadyUsed[idx] = true;
            nbCorrect += 1;
            continue;
        }

        for ( usize partialIdx = 0; partialIdx < s_mastermind.nbPegsPerTurn; ++partialIdx )
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

    while ( nbCorrect-- > 0 ) { pinsTurn[pinIdx++].id = PinId_CORRECT; }
    while ( nbPartial-- > 0 ) { pinsTurn[pinIdx++].id = PinId_PARTIALLY_CORRECT; }
}


static bool is_current_turn_match_solution( void )
{
    struct Pin const *pinsTurn = mastermind_get_pins_at_turn( s_mastermind.currentTurn );
    for ( usize idx = 0; idx < s_mastermind.nbPegsPerTurn; ++idx )
    {
        enum PinId const id = pinsTurn[idx].id;
        if ( id != PinId_CORRECT )
        {
            return false;
        }
    }

    return true;   
}


static bool abandon_game( void )
{
    if ( s_mastermind.gameStatus == GameStatus_IN_PROGRESS )
    {
        s_mastermind.gameStatus = GameStatus_LOST;
        show_solution();
        emit_game_update( GameUpdateType_GAME_FINISHED );
        return true;
    }
    return false;
}


static bool new_game( u8 const nbTurns, u8 const nbPegsPerTurn, enum GameExperience const gameExperience )
{
    // Settings
    s_mastermind.nbTurns = nbTurns;
    s_mastermind.nbPegsPerTurn = nbPegsPerTurn;
    s_mastermind.gameExperience = gameExperience;

    // Game data
    for ( int idx = 0; idx < Mastermind_MAX_TURNS; ++idx )
    {
        reset_pegs_row( s_mastermind.pegs[idx] );
        reset_pins_row( s_mastermind.pins[idx] );
    }
    generate_new_solution( s_mastermind.solution );
    hide_solution();

    // Game logic
    s_mastermind.currentTurn = 1;
    s_mastermind.selectedPeg = PegId_Empty;
    s_mastermind.selectionBarIdx = 0;
    s_mastermind.gameStatus = GameStatus_IN_PROGRESS;

    emit_game_update( GameUpdateType_GAME_NEW );
    return true;
}


bool mastermind_try_consume_input( enum KeyInput const input )
{
    switch( input )
    {
        case KeyInput_N:
        {
            u8 const nbTurns = settings_get_nb_turns();
            u8 const nbPegsPerTurn = settings_get_nb_pieces_per_turn();
            enum GameExperience gameExperience = settings_get_game_experience();
            new_game( nbTurns, nbPegsPerTurn, gameExperience );
            return true;
        }
        case KeyInput_A:
        {
            abandon_game();
            return true;
        }
        case KeyInput_R:
        {
            reset_pegs_row( s_mastermind.pegs[s_mastermind.currentTurn - 1] );
            emit_game_update( GameUpdateType_TURN_RESET );
            return true;
        }
        case KeyInput_ENTER/*KeyInput_V*/:
        {
            if ( !is_current_turn_valid() ) return true;

            generate_feedback_on_current_turn();

            if ( is_current_turn_match_solution() )
            {
                s_mastermind.gameStatus = GameStatus_WON;
                show_solution();
                emit_game_update( GameUpdateType_GAME_FINISHED );
            }
            else if ( s_mastermind.currentTurn == s_mastermind.nbTurns )
            {
                s_mastermind.gameStatus = GameStatus_LOST;
                show_solution();
                emit_game_update( GameUpdateType_GAME_FINISHED );
            }
            else
            {
                s_mastermind.currentTurn += 1;
                s_mastermind.selectionBarIdx = 0;
                emit_game_update( GameUpdateType_NEXT_TURN );
            }
            return true;
        }
        case KeyInput_ARROW_LEFT:
        {
            if ( s_mastermind.selectionBarIdx > 0 )
            {
                s_mastermind.selectionBarIdx -= 1;
                emit_game_update( GameUpdateType_SELECTION_BAR_MOVED );
            }
            return true;
        }
        case KeyInput_ARROW_RIGHT:
        {
            if ( s_mastermind.selectionBarIdx + 1 < s_mastermind.nbPegsPerTurn )
            {
                s_mastermind.selectionBarIdx += 1;
                emit_game_update( GameUpdateType_SELECTION_BAR_MOVED );
            }
            return true;
        }

        case KeyInput_NUMPAD_0 ... KeyInput_NUMPAD_8:
        {
            s_mastermind.selectedPeg = ( input - KeyInput_NUMPAD_0 );
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx].id = s_mastermind.selectedPeg;
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx].hidden = false;
            emit_game_update( GameUpdateType_PEG_ADDED );
            return true;
        }
        case KeyInput_0 ... KeyInput_8:
        {
            s_mastermind.selectedPeg = ( input - KeyInput_0 );
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx].id = s_mastermind.selectedPeg;
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx].hidden = false;
            emit_game_update( GameUpdateType_PEG_ADDED );
            return true;
        }

/*        case KeyInput_ENTER:
        {
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx].id = s_mastermind.selectedPeg;
            s_mastermind.pegs[s_mastermind.currentTurn - 1][s_mastermind.selectionBarIdx].hidden = false;
            emit_game_update( GameUpdateType_PEG_ADDED );
            return true;
        }*/
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

usize mastermind_get_nb_pegs_per_turn( void )
{
    return s_mastermind.nbPegsPerTurn;
}

usize mastermind_get_player_turn( void )
{
    return s_mastermind.currentTurn;
}

u8 mastermind_get_selection_bar_index( struct Mastermind const *mastermind )
{
    return mastermind->selectionBarIdx;
}

enum PegId mastermind_get_selected_peg( struct Mastermind const *mastermind )
{
    return mastermind->selectedPeg;
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

// ////// PEGS FUNCTIONS

enum ConsoleColorFG peg_get_color( enum PegId const id, bool const selected )
{
	switch ( id )
	{
		case PegId_RED:     return ( selected ? ConsoleColorFG_BRIGHT_RED     : ConsoleColorFG_RED     );
		case PegId_GREEN:   return ( selected ? ConsoleColorFG_BRIGHT_GREEN   : ConsoleColorFG_GREEN   );
		case PegId_YELLOW:  return ( selected ? ConsoleColorFG_BRIGHT_YELLOW  : ConsoleColorFG_YELLOW  );
		case PegId_CYAN:    return ( selected ? ConsoleColorFG_BRIGHT_CYAN    : ConsoleColorFG_CYAN    );
		case PegId_MAGENTA: return ( selected ? ConsoleColorFG_BRIGHT_MAGENTA : ConsoleColorFG_MAGENTA );
		case PegId_BLUE:    return ( selected ? ConsoleColorFG_BRIGHT_BLUE    : ConsoleColorFG_BLUE    );
		case PegId_WHITE:   return ( selected ? ConsoleColorFG_BRIGHT_WHITE   : ConsoleColorFG_WHITE   );
         // Perhaps put the \x1b[2m to be darker, this way we won't use white here, but the bright black
		case PegId_BLACK:   return ( selected ? ConsoleColorFG_WHITE          : ConsoleColorFG_BRIGHT_BLACK );
		case PegId_Empty:   return ( selected ? ConsoleColorFG_BRIGHT_BLACK   : ConsoleColorFG_BRIGHT_BLACK );
		default: assert( false );
	}
}
