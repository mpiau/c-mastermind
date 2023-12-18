#include "mastermind_v2.h"

#include <stdio.h>

static inline
struct PegSlot *get_slot_from_board( struct MastermindBoard *board )
{
    return &board->pegSlots[board->currentTurn][board->codePegIdx];
}

static inline
struct MastermindBoard *get_board( struct MastermindV2 *mastermind )
{
    return &mastermind->board;
}


static
void set_slot( struct PegSlot *slot, enum PegSlotType type, u32 value )
{
    slot->type = type;

    switch( slot->type )
    {
        case PegSlotType_KEY_PEG:  slot->keyPeg = value;  break;
        case PegSlotType_CODE_PEG: slot->codePeg = value; break;
        case PegSlotType_EMPTY:    break;
    }
}


static screenpos get_selection_position( struct MastermindBoard const *board )
{
    return (screenpos) {
        .x = board->upLeft.x + 1 + board->currentTurn * 4,
        .y = board->upLeft.y + 1 + board->codePegIdx
    };
}


static void unselect_peg( struct MastermindBoard const *board )
{
    screenpos const pos = get_selection_position( board );
	console_cursor_set_position( pos.y, pos.x );
	wprintf( L" " );
	console_cursor_set_position( pos.y, pos.x + 2 );
	wprintf( L" " );
}

static void select_peg( struct MastermindBoard const *board )
{
    screenpos const pos = get_selection_position( board );
	console_color_fg( ConsoleColorFG_WHITE );
	console_cursor_set_position( pos.y, pos.x );
	wprintf( L"◃" );
	console_cursor_set_position( pos.y, pos.x + 2 );
	wprintf( L"▹" );
}



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
    board->upLeft = (screenpos) { .x = 10, .y = 7 };
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

    mastermind->status = MastermindGameStatus_IN_PROGRESS;
}


void mastermindv2_next_peg_in_row( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;
    unselect_peg( board );

    if ( board->codePegIdx + 1 == mastermind->config.nbCodePegPerTurn )
    {
        board->codePegIdx = 0;
    }
    else
    {
        board->codePegIdx += 1;
    }

    select_peg( board );
}

void mastermindv2_previous_peg_in_row( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;
    unselect_peg( board );

    if ( board->codePegIdx == 0 )
    {
        board->codePegIdx = mastermind->config.nbCodePegPerTurn - 1;
    }
    else
    {
        board->codePegIdx -= 1;
    }

    select_peg( board );
}

void mastermindv2_code_peg_next_color( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;
    struct PegSlot *slot = get_slot_from_board( board );

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
    struct PegSlot *slot = get_slot_from_board( board );

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
    struct PegSlot *slot = get_slot_from_board( board );

    if ( slot->type != PegSlotType_EMPTY )
    {
        slot->type = PegSlotType_EMPTY;
    }
}

bool mastermindv2_next_turn( struct MastermindV2 *mastermind )
{
    struct MastermindBoard *board = &mastermind->board;
    struct PegSlot *currTurn = board->pegSlots[board->currentTurn];

    for ( int i = 0; i < mastermind->config.nbCodePegPerTurn; ++i )
    {
        if ( currTurn[i].type != PegSlotType_CODE_PEG ) return false;
    }

    // Note: Investigate the behaviour when duplicates are authorized. 
	// This code assume that we don't have any duplicates, and I believe that we'll need to handle that differently.
    // Well it doesn't work well if W W R R and we have W and R, it will set 4 partially correct pegs instead of 2. 4 would be in dup allowed
	usize correct = 0;
	usize partial = 0;
	for ( size_t i = 0; i < mastermind->config.nbCodePegPerTurn; ++i )
	{
		if ( currTurn[i].codePeg == board->solution[i].codePeg )
		{
			correct += 1;
			continue;
		}

		// TODO: That second for-loop seems... too much ? Need a better algo, this one is easy-not-pretty
		for ( size_t j = 0; j < mastermind->config.nbCodePegPerTurn; ++j )
		{
			if ( i == j ) continue;
			if ( currTurn[i].codePeg == board->solution[j].codePeg )
			{
				partial += 1;
				break;
			}
		}
	}

    for ( int i = 0; i < mastermind->config.nbCodePegPerTurn; ++i )
    {
        currTurn[i + mastermind->config.nbCodePegPerTurn].type = PegSlotType_KEY_PEG;
        if ( correct > 0 )
        {
            currTurn[i + mastermind->config.nbCodePegPerTurn].keyPeg = KeyPeg_CORRECT;
            correct--;
        }
        else if ( partial > 0 )
        {
            currTurn[i + mastermind->config.nbCodePegPerTurn].keyPeg = KeyPeg_PARTIALLY_CORRECT;
            partial--;
        }
        else
        {
            currTurn[i + mastermind->config.nbCodePegPerTurn].keyPeg = KeyPeg_INCORRECT;
        }
    }

    unselect_peg( board );
    board->currentTurn += 1;
    board->codePegIdx = 0;
    select_peg( board );
    return true;
}

void mastermindv2_draw_selected_peg( struct MastermindV2 *mastermind )
{
    select_peg( &mastermind->board );
}


 // //////// TEMP, COPIED FROM MAIN /////////////////////////////////////////////

 #include "console.h"

static void draw_peg( struct PegSlot const *slot, bool hidden )
{
	if ( slot->type == PegSlotType_EMPTY )
	{
		console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
		wprintf( L"%lc", L'◌' );
	}
	else if ( slot->type == PegSlotType_CODE_PEG )
	{
		if ( hidden )
		{
			console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
			wprintf( L"%lc", L'?' );
		}
		else
		{
			console_color_fg( slot->codePeg + 91 ); // + 91 to match console colors, temp hack
			wprintf( L"%lc", L'⬤' );
		}
	}
	else if ( slot->type == PegSlotType_KEY_PEG )
	{
		console_color_fg( slot->keyPeg ); // + 91 to match console colors, temp hack
		wprintf( L"%lc", slot->keyPeg == KeyPeg_INCORRECT ? L'◌' : L'●' );
	}
}

// ////////////////////////////////////

enum CodePegColorUpdate
{
    CodePegColorUpdate_Next = 1,
    CodePegColorUpdate_Prev = -1
};

static void codepeg_color_update( struct MastermindV2 *mastermind, enum CodePegColorUpdate const update )
{
    struct PegSlot *const slot = get_slot_from_board( get_board( mastermind ) );
    u32 const nbColors = mastermind->config.nbCodePegColors;
    u32 const min = 0;
    u32 const max = nbColors - 1;

    if ( slot->type == PegSlotType_EMPTY )
    {
        set_slot( slot, PegSlotType_CODE_PEG, update == CodePegColorUpdate_Next ? min : max );
    }
    else if ( ( update == CodePegColorUpdate_Prev && slot->codePeg == min )
           || ( update == CodePegColorUpdate_Next && slot->codePeg == max )
        )
    {
        set_slot( slot, PegSlotType_EMPTY, 0 );
    }
    else
    {
        slot->codePeg += update;
    }

    // Now that the peg has been updated, we need to redraw it in the console 
    screenpos pos = get_selection_position( get_board( mastermind ) );
    console_cursor_set_position( pos.y, pos.x + 1 ); // +1 to mvoe it from the left side to the center where the peg is
    draw_peg( slot, false );
}

void mastermind_codepeg_color_prev( struct MastermindV2 *mastermind )
{
    codepeg_color_update( mastermind, CodePegColorUpdate_Prev );
}

void mastermind_codepeg_color_next( struct MastermindV2 *mastermind )
{
    codepeg_color_update( mastermind, CodePegColorUpdate_Next );
}


void mastermind_board_prev_codepeg( struct MastermindV2 *mastermind )
{

}

void mastermind_board_next_codepeg( struct MastermindV2 *mastermind )
{

}
