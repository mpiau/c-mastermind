#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "keyboard_inputs.h"
#include "console/console.h"

#include "game/piece.h"

enum // Constants
{
    Mastermind_MIN_TURNS = 8,
    Mastermind_MAX_TURNS = 20,

    Mastermind_MIN_PIECES_PER_TURN = 4,
    Mastermind_MAX_PIECES_PER_TURN = 6,

    Mastermind_NB_COLORS = PegId_Count, // 4 to 8 ? We would need to handle duplicate pegs. (could be an option as well)

    Mastermind_MAX_CALLBACKS = 4
};

enum GameStatus
{
    GameStatus_IN_PROGRESS,
    GameStatus_LOST,
    GameStatus_WON
};

enum GameExperience
{
    GameExperience_NORMAL,          // The user experience will be the same as the original game.
    GameExperience_LESS_HISTORY,    // Only show the pegs of the 3 previous turns played
    GameExperience_PARTIAL_HISTORY, // For all previous turns, hide 2 random pegs for the rest of the game.
    GameExperience_NO_HISTORY,      // Do not show any peg played in the previous turns
};


struct Mastermind;


// Game update are sent by callback
// Perhaps we could also put some getters for widgets/functions that needs a value on init or anything (like the game status)
// So only the update by callbacks, and then setters to retrieve the data ?

enum GameUpdateType
{
    GameUpdateType_GAME_NEW,
    GameUpdateType_GAME_FINISHED,
    GameUpdateType_TURN_RESET,
    GameUpdateType_NEXT_TURN,
    GameUpdateType_SELECTION_BAR_MOVED,
    GameUpdateType_PEG_ADDED,
    GameUpdateType_PEG_REMOVED,
    // [...]
};

typedef void ( * MastermindCallback )( struct Mastermind const *mastermind, enum GameUpdateType updateType );

bool mastermind_try_consume_input( enum KeyInput input );
bool mastermind_register_update_callback( MastermindCallback const callback );

// getters 

struct Mastermind const *mastermind_get_instance( void );

usize mastermind_get_total_turns( void );
usize mastermind_get_nb_pieces_per_turn( void );
usize mastermind_get_player_turn( void );
u8 mastermind_get_selection_bar_index( struct Mastermind const *mastermind );
enum PegId mastermind_get_selected_peg( struct Mastermind const *mastermind );
bool mastermind_is_game_finished( void );
bool mastermind_is_game_lost( void );
bool mastermind_is_game_won( void );
struct Peg const *mastermind_get_pegs_at_turn( usize turn );
struct Pin const *mastermind_get_pins_at_turn( usize turn );
struct Peg const *mastermind_get_solution( void );


enum ConsoleColorFG peg_get_color( enum PegId id, bool selected );