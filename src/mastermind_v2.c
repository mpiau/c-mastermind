#include "mastermind_v2.h"

void reset_config( struct MastermindConfig *config )
{
    *config = (struct MastermindConfig ) {
        .nbCodePegPerTurn = 4,
        .nbCodePegColors = 6,
        .nbTurns = 12,
        .allowDuplicateCodePeg = false
    };
};


void reset_board( struct MastermindBoard *board )
{
    bool oldHideSolution = board->hideSolution;
    *board = (struct MastermindBoard) {};
    board->hideSolution = oldHideSolution;
};


bool mastermindv2_init( struct MastermindV2 *mastermind )
{
    reset_config( &mastermind->config );
    reset_board( &mastermind->board );
    mastermind->status = MastermindGameStatus_NOT_STARTED;

    return true;
}

void mastermindv2_start_game( struct MastermindV2 *mastermind )
{
    if ( mastermind->status != MastermindGameStatus_NOT_STARTED )
    {
        reset_board( &mastermind->board );
    }

    struct MastermindConfig *config = &mastermind->config;

    struct PegSlot *codemakerSlots = mastermind->board.pegSlots[config->nbTurns];

    bool used[Mastermind_MAX_CODE_PEG_PER_TURN] = {};
    for ( int i = 0; i < config->nbCodePegPerTurn; ++i )
    {
        struct PegSlot *const slot = &mastermind->board.solution[i];
        slot->type = PegSlotType_CODE_PEG;
        do
		{
			slot->keyPeg = rand() % config->nbCodePegColors; // TODO improve that generation line
		} while ( !config->allowDuplicateCodePeg && used[slot->keyPeg] );

		used[slot->keyPeg] = true;
    }

    mastermind->board.pegSlots[0][4].type = PegSlotType_KEY_PEG;
    mastermind->board.pegSlots[0][4].keyPeg = KeyPeg_CORRECT;
    mastermind->board.pegSlots[0][5].type = PegSlotType_KEY_PEG;
    mastermind->board.pegSlots[0][5].keyPeg = KeyPeg_CORRECT;
    mastermind->board.pegSlots[0][6].type = PegSlotType_KEY_PEG;
    mastermind->board.pegSlots[0][6].keyPeg = KeyPeg_PARTIALLY_CORRECT;

    mastermind->status = MastermindGameStatus_IN_PROGRESS;
}

void mastermindv2_next_peg_in_row( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;

    if ( board->currentCodePegIdx + 1 == mastermind->config.nbCodePegPerTurn )
    {
        board->currentCodePegIdx = 0;
    }
    else
    {
        board->currentCodePegIdx += 1;
    }
}

void mastermindv2_previous_peg_in_row( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;

    if ( board->currentCodePegIdx == 0 )
    {
        board->currentCodePegIdx = mastermind->config.nbCodePegPerTurn - 1;
    }
    else
    {
        board->currentCodePegIdx -= 1;
    }
}

void mastermindv2_code_peg_next_color( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;
    struct PegSlot *slot = &board->pegSlots[board->currentTurn][board->currentCodePegIdx];

    if ( slot->type == PegSlotType_EMPTY )
    {
        slot->type = PegSlotType_CODE_PEG;
        slot->codePeg = 0;
        return;
    }

    assert(  slot->type == PegSlotType_CODE_PEG );

    if ( slot->codePeg + 1 == mastermind->config.nbCodePegColors )
    {
        slot->type = PegSlotType_EMPTY;
        slot->codePeg = 0;
    }
    else
    {
        slot->codePeg += 1;
    }
}

void mastermindv2_code_peg_previous_color( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;
    struct PegSlot *slot = &board->pegSlots[board->currentTurn][board->currentCodePegIdx];

    if ( slot->type == PegSlotType_EMPTY )
    {
        slot->type = PegSlotType_CODE_PEG;
        slot->codePeg = mastermind->config.nbCodePegColors - 1;
        return;
    }

    assert(  slot->type == PegSlotType_CODE_PEG );



    if ( slot->codePeg == 0 )
    {
        slot->type = PegSlotType_EMPTY;
        slot->codePeg = 0;
    }
    else
    {
        slot->codePeg -= 1;
    }
}


void mastermindv2_remove_current_codepeg( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;
    struct PegSlot *slot = &board->pegSlots[board->currentTurn][board->currentCodePegIdx];

    if ( slot->type != PegSlotType_EMPTY )
    {
        slot->type = PegSlotType_EMPTY;
    }
}