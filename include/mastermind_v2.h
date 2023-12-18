#pragma once

#include "core_types.h"
#include "core_unions.h"

#include "console.h"

enum
{
    Mastermind_MIN_TURNS = 5,
    Mastermind_MAX_TURNS = 25,

    Mastermind_MIN_CODE_PEG_PER_TURN = 3,
    Mastermind_MAX_CODE_PEG_PER_TURN = 6,

    Mastermind_MIN_PEGS_PER_TURN = Mastermind_MIN_CODE_PEG_PER_TURN * 2, // equals number of code and key pegs
    Mastermind_MAX_PEGS_PER_TURN = Mastermind_MAX_CODE_PEG_PER_TURN * 2,

    Mastermind_MIN_CODE_PEG_COLORS = 4,
    Mastermind_MAX_CODE_PEG_COLORS = 8,
};

enum KeyPeg
{
    KeyPeg_CORRECT = ConsoleColorFG_BRIGHT_RED,
    KeyPeg_PARTIALLY_CORRECT = ConsoleColorFG_BRIGHT_WHITE,
    KeyPeg_INCORRECT = ConsoleColorFG_BRIGHT_BLACK
};

enum CodePeg
{
    CodePeg_1, 
    // ...
};

enum MastermindGameStatusV2
{
	MastermindGameStatus_NOT_STARTED,
	MastermindGameStatus_IN_PROGRESS,
	MastermindGameStatus_FINISHED,
};


struct MastermindConfig
{
	u8 nbCodePegPerTurn; // min 3 max 6
	u8 nbCodePegColors;  // min 3 max 8
	u8 nbTurns;          // min 8 max 25
	bool allowDuplicateCodePeg;
};

enum PegSlotType
{
    PegSlotType_EMPTY,
    PegSlotType_CODE_PEG,
    PegSlotType_KEY_PEG,
};

struct PegSlot
{
    enum PegSlotType type;
    union {
        enum CodePeg codePeg;
        enum KeyPeg  keyPeg;
    };
};

struct MastermindBoard
{
    struct PegSlot pegSlots[Mastermind_MAX_TURNS][Mastermind_MAX_PEGS_PER_TURN];
    struct PegSlot solution[Mastermind_MAX_CODE_PEG_PER_TURN];
    bool hideSolution;
    u8 currentTurn;
    u8 codePegIdx;
    screenpos upLeft;
};

struct MastermindV2
{
	struct MastermindConfig config;
	struct MastermindBoard  board;
    enum MastermindGameStatusV2 status;
};


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

void draw_entire_game( struct MastermindV2 *mastermind );

/*
enum MastermindTurnStatus
{
    MastermindTurnStatus_SUCCESS,
    MastermindTurnStatus_ERROR_EMPTY_SLOT,
    MastermindTurnStatus_ERROR_DUPLICATE_PEG
};

enum MastermindTurnStatus mastermind_board_finish_turn( struct MastermindV2 *mastermind );

void mastermind_board_prev_codepeg( struct MastermindV2 *mastermind );
void mastermind_board_next_codepeg( struct MastermindV2 *mastermind );
*/