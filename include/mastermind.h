#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "keyboard_inputs.h"
#include "console.h"

enum // Constants
{
    Mastermind_MIN_TURNS = 8,
    Mastermind_MAX_TURNS = 16,

    Mastermind_MIN_PEGS_PER_TURN = 4,
    Mastermind_MAX_PEGS_PER_TURN = 6,

    Mastermind_MIN_PINS_PER_TURN = Mastermind_MIN_PEGS_PER_TURN,
    Mastermind_MAX_PINS_PER_TURN = Mastermind_MAX_PEGS_PER_TURN,

    Mastermind_NB_COLORS = 8,

    Mastermind_MAX_CALLBACKS = 4
};

enum PinId
{
    PinId_CORRECT,           // Correct color + correct position
    PinId_PARTIALLY_CORRECT, // Correct color + incorrect position
    PinId_INCORRECT,         // both incorrect

    PinId_Count,
};

enum PegId
{
    PegId_RED,
    PegId_GREEN,
    PegId_YELLOW,
    PegId_CYAN,
    PegId_MAGENTA,
    PegId_BLUE,
    PegId_WHITE,
    PegId_BLACK,

    PegId_Count,
    PegId_Empty = PegId_Count
};
static_assert( PegId_Count >= Mastermind_NB_COLORS );

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

struct Peg
{
    enum PegId id;
    bool hidden;
};

struct Pin
{
    enum PinId id;
    bool hidden;
};

struct Mastermind;


// Game update are sent by callback
// Perhaps we could also put some getters for widgets/functions that needs a value on init or anything (like the game status)
// So only the update by callbacks, and then setters to retrieve the data ?

enum GameUpdateType
{
    GameUpdateType_GAME_STATUS,
    GameUpdateType_TURN_RESET,
    GameUpdateType_NEXT_TURN,
    // [...]
};

typedef void ( * MastermindCallback )( struct Mastermind const *mastermind, enum GameUpdateType updateType );

bool mastermind_try_consume_input( enum KeyInput input );
bool mastermind_register_update_callback( MastermindCallback const callback );

// getters 

struct Mastermind const *mastermind_get_instance( void );

u8   mastermind_get_total_nb_turns( struct Mastermind const *mastermind );
u8   mastermind_get_nb_pegs_per_turn( struct Mastermind const *mastermind );
u8   mastermind_get_current_turn( struct Mastermind const *mastermind );
bool mastermind_is_game_finished( struct Mastermind const *mastermind );
bool mastermind_is_game_lost( struct Mastermind const *mastermind );
bool mastermind_is_game_won( struct Mastermind const *mastermind );
struct Peg const *mastermind_get_pegs_at_turn( struct Mastermind const *mastermind, u8 turn );
struct Pin const *mastermind_get_pins_at_turn( struct Mastermind const *mastermind, u8 turn );

enum ConsoleColorFG peg_get_color( enum PegId id, bool selected );
enum ConsoleColorFG pin_get_color( enum PinId id );

/*
bool mastermindv2_init( struct MastermindV2 *mastermind );
void mastermindv2_start_game( struct MastermindV2 *mastermind );

void mastermindv2_next_peg_in_row( struct MastermindV2 *mastermind );
void mastermindv2_previous_peg_in_row( struct MastermindV2 *mastermind );

void mastermindv2_remove_current_codepeg( struct MastermindV2 *mastermind );
bool mastermindv2_next_turn( struct MastermindV2 *mastermind );

void mastermindv2_draw_selected_peg( struct MastermindV2 *mastermind );


// Next version ///////////////////////////////////////////

void mastermind_codepeg_color_prev( struct MastermindV2 *mastermind );
void mastermind_codepeg_color_next( struct MastermindV2 *mastermind );

enum KeyInput;
bool mastermind_try_consume_input( struct MastermindV2 *mastermind, enum KeyInput input );

void draw_entire_game( struct MastermindV2 *mastermind, screenpos screenSize );
*/