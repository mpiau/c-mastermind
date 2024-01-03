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
    struct Peg pegs[Mastermind_MAX_TURNS][Mastermind_MAX_PEGS_PER_TURN];
    struct Pin pins[Mastermind_MAX_TURNS][Mastermind_MAX_PEGS_PER_TURN];
    struct Peg solution[Mastermind_MAX_PEGS_PER_TURN];

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
    for ( int idx = 0; idx < Mastermind_MAX_PEGS_PER_TURN; ++idx )
    {
        pegs[idx].id = PegId_Empty;
        pegs[idx].hidden = false;
    }
}

static void reset_pins_row( struct Pin *const pins )
{
    for ( int idx = 0; idx < Mastermind_MAX_PINS_PER_TURN; ++idx )
    {
        pins[idx].id = PinId_INCORRECT;
        pins[idx].hidden = false;
    }
}

static void generate_new_solution( struct Peg *const pegs )
{
    // Note: For the moment, we aren't supporting having duplicated pegs in the same row.
    // So this generation is explicitly checking to have unique pegs in the generated solution.
    bool pegsUsed[Mastermind_NB_COLORS] = {};

    for ( int idx = 0; idx < Mastermind_MAX_PEGS_PER_TURN; ++idx )
    {
        do
		{
            pegs[idx].id = rand() % Mastermind_NB_COLORS;
		} while ( pegsUsed[pegs[idx].id] );

		pegsUsed[pegs[idx].id] = true;
    }
}


static void hide_solution()
{
    for ( int idx = 0; idx < Mastermind_MAX_PEGS_PER_TURN; ++idx )
    {
            s_mastermind.solution[idx].hidden = true;
    }
}


static void show_solution()
{
    for ( int idx = 0; idx < Mastermind_MAX_PEGS_PER_TURN; ++idx )
    {
            s_mastermind.solution[idx].hidden = false;
    }
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
            u8 const nbTurns = settings_get_number_turns();
            u8 const nbPegsPerTurn = settings_get_pegs_per_turn();
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
        case KeyInput_V:
        {
            // TODO check if the turn can be validated first
            // Then generate feedback for the turn and either increment turn, or Win/Lost condition depending of the result
            
            // emit_game_update( GameUpdateType_NEXT_TURN ); for turn validation
            // emit_game_update( GameUpdateType_GAME_FINISHED ); for win-lost situation
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


u8 mastermind_get_total_nb_turns( struct Mastermind const *mastermind )
{
    return mastermind->nbTurns;
}

u8   mastermind_get_nb_pegs_per_turn( struct Mastermind const *mastermind )
{
    return mastermind->nbPegsPerTurn;
}

u8 mastermind_get_current_turn( struct Mastermind const *mastermind )
{
    return mastermind->currentTurn;
}

u8 mastermind_get_selection_bar_index( struct Mastermind const *mastermind )
{
    return mastermind->selectionBarIdx;
}

bool mastermind_is_game_finished( struct Mastermind const *mastermind )
{
    return mastermind_is_game_lost( mastermind ) || mastermind_is_game_won( mastermind );
}

bool mastermind_is_game_lost( struct Mastermind const *mastermind )
{
    return mastermind->gameStatus == GameStatus_LOST;
}

bool mastermind_is_game_won( struct Mastermind const *mastermind )
{
    return mastermind->gameStatus == GameStatus_WON;
}

struct Mastermind const *mastermind_get_instance( void )
{
    return &s_mastermind;
}

struct Peg const *mastermind_get_pegs_at_turn( struct Mastermind const *mastermind, u8 const turn )
{
    assert( turn > 0 && turn <= mastermind->nbTurns );
    return mastermind->pegs[turn - 1];
}

struct Pin const *mastermind_get_pins_at_turn( struct Mastermind const *mastermind, u8 const turn )
{
    assert( turn > 0 && turn <= mastermind->nbTurns );
    return mastermind->pins[turn - 1];
}

struct Peg const *mastermind_get_solution( struct Mastermind const *mastermind )
{
    return mastermind->solution;
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


// //// PINS FUNCTIONS

enum ConsoleColorFG pin_get_color( enum PinId const id )
{
	switch ( id )
	{
		case PinId_CORRECT:			  return ConsoleColorFG_RED;
		case PinId_PARTIALLY_CORRECT: return ConsoleColorFG_WHITE;
		case PinId_INCORRECT:         return ConsoleColorFG_BRIGHT_BLACK;
		default: assert( false );
	}
}



/*
// DRAW GAME /////////////////////////////////////////////////////////////////////
// DRAW GAME /////////////////////////////////////////////////////////////////////

static
void board_draw_horizontal_borders( vec2u16 const beginCoords, bool const isTopBorder, u16 const nbTurns )
{
	utf16 const leftCorner  = isTopBorder ? UTF16C_DoubleDownRight : UTF16C_DoubleUpRight;
	utf16 const rightCorner = isTopBorder ? UTF16C_DoubleDownLeft : UTF16C_DoubleUpLeft;

	utf16 const horizLineDelim     = isTopBorder ? UTF16C_DoubleHorizSingleDown : UTF16C_DoubleHorizSingleUp;
	utf16 const horizLineDelimWide = isTopBorder ? UTF16C_DoubleHorizAndDown : UTF16C_DoubleHorizAndUp;
	utf16 const horizLine = UTF16C_DoubleHoriz;

	u16 const WIDTH_PER_TURN          = 4;
	u16 const TOTAL_WIDTH             = WIDTH_PER_TURN * nbTurns;
	u16 const TOTAL_WIDTH_WITH_RESULT = TOTAL_WIDTH + WIDTH_PER_TURN;

	console_cursor_set_position( beginCoords.y, beginCoords.x );

	console_draw( L"%lc", leftCorner );
	for ( int x = 1; x < TOTAL_WIDTH_WITH_RESULT; ++x )
	{
		if ( x % WIDTH_PER_TURN == 0 )
		{
			console_draw( L"%lc", x == TOTAL_WIDTH ? horizLineDelimWide : horizLineDelim );
			continue;
		}
		console_draw( L"%lc", horizLine );
	}
	console_draw( L"%lc", rightCorner );	
}

void draw_horizontal_border( vec2u16 const beginCoords, bool const isTopBorder, u16 const nbTurns )
{
	utf16 const leftCorner  = isTopBorder ? UTF16C_DoubleDownRight : UTF16C_DoubleUpRight;
	utf16 const rightCorner = isTopBorder ? UTF16C_DoubleDownLeft : UTF16C_DoubleUpLeft;

	utf16 const horizLineDelim     = isTopBorder ? UTF16C_DoubleHorizSingleDown : UTF16C_DoubleHorizSingleUp;
	utf16 const horizLineDelimWide = isTopBorder ? UTF16C_DoubleHorizAndDown : UTF16C_DoubleHorizAndUp;
	utf16 const horizLine = UTF16C_DoubleHoriz;

	u16 const WIDTH_PER_TURN          = 4;
	u16 const TOTAL_WIDTH             = WIDTH_PER_TURN * nbTurns;
	u16 const TOTAL_WIDTH_WITH_RESULT = TOTAL_WIDTH + WIDTH_PER_TURN;

	console_cursor_set_position( beginCoords.y, beginCoords.x );

	console_draw( L"%lc", leftCorner );
	for ( int x = 1; x < TOTAL_WIDTH_WITH_RESULT; ++x )
	{
		if ( x % WIDTH_PER_TURN == 0 )
		{
			console_draw( L"%lc", x == TOTAL_WIDTH ? horizLineDelimWide : horizLineDelim );
			continue;
		}
		console_draw( L"%lc", horizLine );
	}
	console_draw( L"%lc", rightCorner );	
}


void draw_center_board( vec2u16 const screenPos, u16 const pegsPerRow, u16 const nbTurns )
{
	u16 const WIDTH_PER_TURN          = 4;
	u16 const TOTAL_WIDTH             = WIDTH_PER_TURN * nbTurns;
	u16 const TOTAL_WIDTH_WITH_RESULT = TOTAL_WIDTH + WIDTH_PER_TURN;

	for ( int y = 0; y < pegsPerRow; ++y )
	{
		for ( int x = 0; x <= TOTAL_WIDTH_WITH_RESULT; x += WIDTH_PER_TURN )
		{
			console_cursor_set_position( screenPos.y + y, screenPos.x + x );
			if ( x == 0 || x == TOTAL_WIDTH || x == TOTAL_WIDTH_WITH_RESULT )
			{
				console_draw( L"%lc", UTF16C_DoubleVert );
				continue;
			}
			console_draw( L"%lc", UTF16C_LightVert );
		}
	}
}


void draw_gameboard( vec2u16 upLeftPos, struct MastermindConfig const *config )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

    board_draw_horizontal_borders( upLeftPos, true, config->nbTurns );

	upLeftPos.y += 1;
	draw_center_board( upLeftPos, config->nbCodePegPerTurn, config->nbTurns );

	upLeftPos.y += config->nbCodePegPerTurn;
    board_draw_horizontal_borders( upLeftPos, false, config->nbTurns );
}


static void board_draw_pegs( struct PegSlot const *slot, bool hidden )
{
	if ( slot->type == PegSlotType_EMPTY )
	{
		console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
		console_draw( L"%lc", UTF16C_SmallDottedCircle );
	}
	else if ( slot->type == PegSlotType_CODE_PEG )
	{
		if ( hidden )
		{
			console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
			console_draw( L"%lc", L'?' );
		}
		else
		{
			console_color_fg( slot->codePeg + 91 ); // + 91 to match console colors, temp hack
			console_draw( L"%lc", UTF16C_BigFilledCircle );
		}
	}
	else if ( slot->type == PegSlotType_KEY_PEG )
	{
		console_color_fg( slot->keyPeg ); // + 91 to match console colors, temp hack
		console_draw( L"%lc", slot->keyPeg == KeyPeg_INCORRECT ? UTF16C_SmallDottedCircle : UTF16C_SmallFilledCircle );
	}
}


void board_draw_solution( struct MastermindConfig const *config, struct MastermindBoard const *board )
{
    screenpos screenPos = (screenpos) {
        .x = board->upLeft.x + 2 + 4 * config->nbTurns,
        .y = board->upLeft.y + 1,
    };

	for ( int y = 0; y < config->nbCodePegPerTurn; ++y )
	{
		console_cursor_set_position( screenPos.y + y, screenPos.x );
		board_draw_pegs( &board->solution[y], board->hideSolution );
	}   
}


void board_draw_last_turn_feedback( struct MastermindConfig const *config, struct MastermindBoard const *board )
{
    u32 const currTurn = board->currentTurn - 1;

    screenpos screenPos = (screenpos) {
        .x = board->upLeft.x + 2 + 4 * currTurn,
        .y = board->upLeft.y + config->nbCodePegPerTurn + 2, // + 2 == borders
    };

	for ( int y = 0; y < ( config->nbCodePegPerTurn + 1 ) / 2; ++y )
	{
		console_cursor_set_position( screenPos.y + y, screenPos.x - 1 );
		board_draw_pegs( &board->pegSlots[currTurn][y * 2 + config->nbCodePegPerTurn], false );
		if ( y + 1 < config->nbCodePegPerTurn )
		{
			console_cursor_set_position( screenPos.y + y, screenPos.x );
			board_draw_pegs( &board->pegSlots[currTurn][y * 2 + config->nbCodePegPerTurn + 1], false );
		}
	}
}


void draw_gameboard_content( vec2u16 screenPos, struct MastermindConfig const *config, struct MastermindBoard const *board )
{
	u16 const WIDTH_PER_TURN          = 4;
	u16 const TOTAL_WIDTH             = WIDTH_PER_TURN * config->nbTurns;
	u16 const TOTAL_WIDTH_WITH_RESULT = TOTAL_WIDTH + WIDTH_PER_TURN;

	// Turn-pegs
	for ( int turn = 0; turn < config->nbTurns; ++turn )
	{
		for ( int y = 0; y < config->nbCodePegPerTurn; ++y )
		{
			console_cursor_set_position( screenPos.y + y, screenPos.x + ( turn * WIDTH_PER_TURN ) );
			board_draw_pegs( &board->pegSlots[turn][y], false );
		}
	}

	// Solution
    board_draw_solution( config, board );

	// Feedback
	for ( int turn = 0; turn < board->currentTurn; ++turn )
	{
		for ( int y = 0; y < ( config->nbCodePegPerTurn + 1 ) / 2; ++y )
		{
			console_cursor_set_position( screenPos.y + y + config->nbCodePegPerTurn + 1, ( screenPos.x - 1 ) + ( turn * WIDTH_PER_TURN ) );
			board_draw_pegs( &board->pegSlots[turn][y * 2 + config->nbCodePegPerTurn], false );
			if ( y + 1 < config->nbCodePegPerTurn )
			{
				console_cursor_set_position( screenPos.y + y + config->nbCodePegPerTurn + 1, screenPos.x + ( turn * WIDTH_PER_TURN ) );
				board_draw_pegs( &board->pegSlots[turn][y * 2 + config->nbCodePegPerTurn + 1], false );
			}
		}
	}
}


// DRAW SUMMARY GAME //////////////////////////////////////////////////////////////
// DRAW SUMMARY GAME //////////////////////////////////////////////////////////////

void summary_draw_borders( screenpos pos, struct MastermindV2 const *mastermind )
{
    u32 const nbTurns = mastermind->config.nbTurns;
    u32 const nbPegsPerTurn = mastermind->config.nbCodePegPerTurn;
    // 4 -> border + space each side, + 2 -> middle with - and space
    u32 const borderWidth = 4 + 3 * nbPegsPerTurn + 2;

    // Upper border
    console_cursor_set_position( pos.y, pos.x );
    console_color_fg( ConsoleColorFG_WHITE );
    int prefixSize = console_draw( L"┌" );
    console_color_fg( ConsoleColorFG_GREEN );
    prefixSize += console_draw( L" Summary " );
    console_color_fg( ConsoleColorFG_WHITE );
    for ( int i = 0; i < borderWidth - 1 - prefixSize; ++ i )
    {
        console_draw( L"─" );
    }
    console_draw( L"┐" );
    pos.y += 1;

    // Middle border
    for ( int i = 0; i < nbTurns; i++ )
    {
        console_cursor_set_position( pos.y + i, pos.x );
        console_draw( L"│" );
        console_cursor_set_position( pos.y + i, pos.x + 2 * nbPegsPerTurn + 2 );
        console_draw( L"-" );
        console_cursor_set_position( pos.y + i, pos.x + borderWidth - 1 );
        console_draw( L"│" );
    }
    pos.y += nbTurns;

    utf16 title[] = L"MASTERMIND";
    u16 nbSpacesEachSide = ( borderWidth - ARR_COUNT( title ) ) / 2;

   	console_cursor_set_position( pos.y, pos.x );
	console_draw( L"│" );
    console_cursor_move_right_by( nbSpacesEachSide );
	console_color_fg( ConsoleColorFG_YELLOW );
	console_draw( L"MASTERMIND" );
	console_color_fg( ConsoleColorFG_WHITE );
    console_cursor_set_position( pos.y, pos.x + borderWidth - 1 );
	console_draw( L"│", nbSpacesEachSide );

    pos.y += 1;
    console_cursor_set_position( pos.y, pos.x );
	console_draw( L"│" );
    console_cursor_move_right_by( borderWidth - 2 );
	console_draw( L"│" );

    pos.y += 1;
    console_cursor_set_position( pos.y, pos.x );
    console_draw( L"└" );
    for ( int i = 0; i < borderWidth - 2; ++ i )
    {
        console_draw( L"─" );
    }
    console_draw( L"┘" );
}


void summary_draw_content( screenpos pos, struct MastermindV2 const *mastermind )
{
    u32 const nbTurns = mastermind->config.nbTurns;
    u32 const nbPegsPerTurn = mastermind->config.nbCodePegPerTurn;
    // 4 -> border + space each side, + 2 -> middle with - and space
    u32 const borderWidth = 4 + 3 * nbPegsPerTurn + 2;
    
    struct MastermindConfig const *config = &mastermind->config;
    struct MastermindBoard const *board = &mastermind->board;
    pos.y += 1; // to get from borders to first line
    pos.x += 2; // to get from border + space to first peg

    for ( int y = 0; y < config->nbTurns; ++y )
    {
        for ( int x = 0; x < config->nbCodePegPerTurn; ++x )
        {
			console_cursor_set_position( pos.y + y, pos.x + ( x * 2 ) );
			board_draw_pegs( &board->pegSlots[y][x], false );
        }

        console_cursor_move_right_by( 3 );
        for ( int x = 0; x < config->nbCodePegPerTurn; ++x )
        {
			board_draw_pegs( &board->pegSlots[y][x + config->nbCodePegPerTurn], false );
        }
    }

    console_cursor_set_position( pos.y + config->nbTurns + 1, pos.x );

    u16 feedbackSpace = config->nbCodePegPerTurn + ( config->nbCodePegPerTurn - 1 ) * 2; // peg + 2 spaces between them
    u16 nbSpacesEachSide = ( borderWidth - feedbackSpace - 2 ) / 2; // - 2 because of the += 2 in the x at the beginning
    console_cursor_move_right_by( nbSpacesEachSide - 1 );

    for ( int x = 0; x < config->nbCodePegPerTurn; ++x )
    {
		board_draw_pegs( &board->solution[x], mastermind->board.hideSolution );
		console_cursor_move_right_by( 2 );
    }
}


void summary_draw( screenpos const screenSize, struct MastermindV2 const *mastermind )
{
   u32 const nbTurns = mastermind->config.nbTurns;
   u32 const nbPegsPerTurn = mastermind->config.nbCodePegPerTurn;
    // 4 -> border + space each side, + 2 -> middle with - and space
   u32 const borderWidth = 4 + 3 * nbPegsPerTurn + 2;

    // Upper border
   screenpos upLeft = (screenpos){ .x = screenSize.x - borderWidth, .y = 2 };

    summary_draw_borders( upLeft, mastermind );
    summary_draw_content( upLeft, mastermind );
}

// /////////////////////////////////////////////////////////////////////
void draw_entire_game( struct MastermindV2 *mastermind, screenpos const screenSize )
{
	vec2u16 upLeft = mastermind->board.upLeft;
	u16 const nbTurns = mastermind->config.nbTurns;
	u16 const pegsPerRow = mastermind->config.nbCodePegPerTurn;

	draw_gameboard( upLeft, &mastermind->config );//pegsPerRow, nbTurns );

	upLeft.x += 2;
	upLeft.y += 1;

	// Need to take actual data to set : 
	// The colored pegs / placeholder
	// ? at the end or solution if finished
	// The feedback row up to current turn (excluded)
	draw_gameboard_content( upLeft, &mastermind->config, &mastermind->board );
    summary_draw( screenSize, mastermind );
	mastermindv2_draw_selected_peg( mastermind );
}
// /////////////////////////////////////////////////////////////////////

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
	console_draw( L" " );
	console_cursor_set_position( pos.y, pos.x + 2 );
	console_draw( L" " );
}

static void select_peg( struct MastermindBoard const *board )
{
    screenpos const pos = get_selection_position( board );
	console_color_fg( ConsoleColorFG_WHITE );
	console_cursor_set_position( pos.y, pos.x );
	console_draw( L"◃" );
	console_cursor_set_position( pos.y, pos.x + 2 );
	console_draw( L"▹" );
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
		console_draw( L"%lc", L'◌' );
	}
	else if ( slot->type == PegSlotType_CODE_PEG )
	{
		if ( hidden )
		{
			console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
			console_draw( L"%lc", L'?' );
		}
		else
		{
			console_color_fg( slot->codePeg + 91 ); // + 91 to match console colors, temp hack
			console_draw( L"%lc", L'⬤' );
		}
	}
	else if ( slot->type == PegSlotType_KEY_PEG )
	{
		console_color_fg( slot->keyPeg ); // + 91 to match console colors, temp hack
		console_draw( L"%lc", slot->keyPeg == KeyPeg_INCORRECT ? L'◌' : L'●' );
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


bool mastermind_try_consume_input( struct MastermindV2 *mastermind, enum KeyInput input )
{
    if ( mastermind->status == MastermindGameStatus_PAUSED && input == KeyInput_P )
    {
        mastermind->status = MastermindGameStatus_IN_PROGRESS;
        console_cursor_set_position( 2, 1 );
        console_draw( L"            ");
        return true;
    }
    else if ( mastermind->status != MastermindGameStatus_IN_PROGRESS )
    {
        // If the game isn't currently running, do not 
        return false;
    }
    // We need to prevent having same keys on 2 differents actions on the entire game

    switch ( input )
    {
		case KeyInput_P:
        {
			mastermind->status = MastermindGameStatus_PAUSED;
            console_cursor_set_position( 2, 1 );
            console_draw( L"-- Paused --");
            return true;
        }
		case KeyInput_ARROW_LEFT:
        {
			mastermind_codepeg_color_prev( mastermind );
            return true;
        }
		case KeyInput_ARROW_RIGHT:
        {
			mastermind_codepeg_color_next( mastermind );
            return true;
        }
        case KeyInput_H:
        {
            mastermind->board.hideSolution = !mastermind->board.hideSolution;
            board_draw_solution( &mastermind->config, &mastermind->board );
            return true;
        }
        case KeyInput_D:
        {
            mastermindv2_remove_current_codepeg( mastermind );
            screenpos pos = get_selection_position( &mastermind->board );
            console_cursor_set_position( pos.y, pos.x + 1 );
            board_draw_pegs( get_slot_from_board( &mastermind->board ), false );
            return true;
        }
        case KeyInput_ARROW_DOWN:
        {
			mastermindv2_next_peg_in_row( mastermind );
            return true;
        }
        case KeyInput_ARROW_UP:
        {
			mastermindv2_previous_peg_in_row( mastermind );
            return true;
        }
        case KeyInput_ENTER:
        {
            mastermindv2_next_turn( mastermind );
            board_draw_last_turn_feedback( &mastermind->config, &mastermind->board );
            return true;
        }
        case KeyInput_R:
		{
            unselect_peg( &mastermind->board );
			mastermindv2_start_game( mastermind );
            screenpos upLeft = mastermind->board.upLeft;
            upLeft.x += 2;
            upLeft.y += 1;
			draw_gameboard_content( upLeft, &mastermind->config, &mastermind->board );
            select_peg( &mastermind->board );
            return true;
        }
    }

    return false;
}*/