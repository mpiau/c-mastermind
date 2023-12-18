// Mastermind game in C
#include "game_menus.h"
#include "mastermind_v2.h"
#include "terminal.h"
#include "console.h"
#include "characters_list.h"
#include "core_unions.h"
#include "fps_counter.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <windows.h>


char const *get_game_name( void )
{
	 // I used https://edukits.co/text-art/ for the ASCII generation.
	 // I needed to add some backslashes afterwards to not break the output with printf.

	static char buffer[1024] = {};

	if ( buffer[0] == '\0' )
	{
		snprintf( buffer, ARR_COUNT( buffer),
			"%s      __  __           _                      %s_%s           _      \n"
			"     |  \\/  | __ _ ___| |_ ___ _ __ _ __ ___ %s(_)%s_ __   __| |     \n"
			"     | |\\/| |/ _` / __| __/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` |     \n"
			"     | |  | | (_| \\__ \\ ||  __/ |  | | | | | | | | | | (_| |     \n"
			"     |_|  |_|\\__,_|___/\\__\\___|_|  |_| |_| |_|_|_| |_|\\__,_|     \n"
			"%s\n",
			S_COLOR_STR[TERM_BOLD_RED], S_COLOR_STR[TERM_BOLD_GREEN], S_COLOR_STR[TERM_BOLD_RED],
			S_COLOR_STR[TERM_BOLD_GREEN], S_COLOR_STR[TERM_BOLD_RED],
			S_COLOR_STR_RESET
		);
	}

	return buffer;
}


// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences Useful
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797 


typedef union vec2u32
{
	struct { u32 x; u32 y; };
	u32 data[2];
} vec2u32;


bool vec2u32_equals( vec2u32 const lhs, vec2u32 const rhs )
{
	return lhs.x == rhs.x && rhs.y == lhs.y;
}


enum ConsoleScreenState
{
	ConsoleScreenState_CURRENT,
	ConsoleScreenState_OLD,

	ConsoleScreenState_COUNT
};


struct ConsoleScreen
{
	HANDLE	handle;
	vec2u32 minSupportedSize;
	vec2u32 size[ConsoleScreenState_COUNT];
	bool	rewriteNeeded;
};


bool console_screen_init( struct ConsoleScreen *const cscreen )
{
	*cscreen = ( struct ConsoleScreen ) {};
	cscreen->handle = GetStdHandle( STD_OUTPUT_HANDLE );

	cscreen->minSupportedSize.x = 65;
	cscreen->minSupportedSize.y = 25;
	cscreen->rewriteNeeded = true;

	return cscreen->handle != INVALID_HANDLE_VALUE;
}


bool console_screen_has_been_resized( struct ConsoleScreen *const cscreen )
{
	return !vec2u32_equals( cscreen->size[ConsoleScreenState_OLD], cscreen->size[ConsoleScreenState_CURRENT] );
}


void console_screen_update_state( struct ConsoleScreen *const cscreen )
{
	cscreen->size[ConsoleScreenState_OLD] = cscreen->size[ConsoleScreenState_CURRENT];

	CONSOLE_SCREEN_BUFFER_INFO csinfo;
	GetConsoleScreenBufferInfo( cscreen->handle, &csinfo );	// todo handle return code

	cscreen->size[ConsoleScreenState_CURRENT] = (vec2u32) { .x = csinfo.dwSize.X, .y = csinfo.dwSize.Y };

	if ( console_screen_has_been_resized( cscreen ) )
	{
		cscreen->rewriteNeeded = true;
	}

	printf( "cursor pos %i and %i\n", csinfo.dwCursorPosition.X, csinfo.dwCursorPosition.Y );
	GetConsoleScreenBufferInfo( cscreen->handle, &csinfo );	// todo handle return code
	printf( "cursor pos %i and %i\n", csinfo.dwCursorPosition.X, csinfo.dwCursorPosition.Y );
}


bool console_screen_is_too_small( struct ConsoleScreen *const cscreen )
{
	vec2u32 const currSize = cscreen->size[ConsoleScreenState_CURRENT];
	vec2u32 const minSize = cscreen->minSupportedSize;

	return currSize.x < minSize.x || currSize.y < minSize.y;
}


/*vec2u32 console_screen_get_size( struct ConsoleScreen *const cscreen )
{
	return cscreen->size[ConsoleScreenState_CURRENT];
}*/


void write_too_small_screen_warning( struct ConsoleScreen *const cscreen )
{
	vec2u32 const screenSize = cscreen->size[ConsoleScreenState_CURRENT];
	bool const isXTooSmall = screenSize.x < cscreen->minSupportedSize.x;
	bool const isYTooSmall = screenSize.y < cscreen->minSupportedSize.y;

	char const *xColor = isXTooSmall ? S_COLOR_STR[TERM_RED] : S_COLOR_STR[TERM_GREEN];
	char const *yColor = isYTooSmall ? S_COLOR_STR[TERM_RED] : S_COLOR_STR[TERM_GREEN];
	
	printf(
		"\033[H\x1B[2J\x1B[3J" // H -> cursor top, 2J == clean current screen and 3J == clean scroll history
		"%s/!\\ Screen too small for the game /!\\%s\n"
		"Minimum required: %ux%u\n"
		"Current         : %s%u%sx%s%u%s",
		S_COLOR_STR[TERM_BOLD_YELLOW], S_COLOR_STR_RESET,
		cscreen->minSupportedSize.x, cscreen->minSupportedSize.y,
		xColor, screenSize.x, S_COLOR_STR_RESET, yColor, screenSize.y, S_COLOR_STR_RESET
	);

	cscreen->rewriteNeeded = false;
}


void game_menu_tile( char buffer[static 256], char const *name, bool selected )
{
	if ( selected )
	{
		sprintf( buffer,
		"%s                  ____________________________\n"
		"                 |                            |\n"
		"                 |%s|\n"
		"                 \\____________________________/%s\n", S_COLOR_STR[TERM_BOLD_WHITE], name, S_COLOR_STR_RESET );
	}
	else
	{
		sprintf( buffer, "\n"
		"\n"
		"                  %s\n"
		"\n", name );		
	}	
}

static int s_currSelect = 0;


void write_hardcoded_game( struct ConsoleScreen *const cscreen, int selectMoved )
{
	char tiles[3][256] = {};

	int temp = s_currSelect;
	temp += selectMoved;
	if ( temp < 0 ) temp = 0;
	if ( temp > 2 ) temp = 2;

	if ( temp != s_currSelect ) s_currSelect = temp;
	else if ( !cscreen->rewriteNeeded ) return;

	game_menu_tile( tiles[0], "        SINGLEPLAYER        ", s_currSelect == 0 );
	game_menu_tile( tiles[1], "     MULTIPLAYER (Local)    ", s_currSelect == 1 );
	game_menu_tile( tiles[2], "           QUIT             ", s_currSelect == 2 );

	printf(
		"\033[H\x1B[2J\x1B[3J%s\n"
		"%s\n%s\n%s\n"
		, get_game_name(),
		tiles[0], tiles[1], tiles[2]
	);

	cscreen->rewriteNeeded = false;
}


bool read_next_input( HANDLE const handle, enum KeyInput *input )
{
	DWORD cNumRead;
	INPUT_RECORD irInBuf;
	ReadConsoleInput( handle, &irInBuf, 1, &cNumRead );

	// We are only checking key events for this game, doesn't need to handle more cases.
	if ( irInBuf.EventType != KEY_EVENT ) return false;
	if ( !irInBuf.Event.KeyEvent.bKeyDown ) return false;

	return key_input_from_u32( irInBuf.Event.KeyEvent.wVirtualKeyCode, input );
}

// Get cursor position
// ESC [ 6 n 	DECXCPR 	Report Cursor Position 	Emit the cursor position as: ESC [ <r> ; <c> R Where <r> = cursor row and <c> = cursor column

// https://www.w3schools.com/charsets/ref_utf_box.asp 
// https://www.compart.com/en/unicode/U+25CF

#define NB_TURNS 12
#define SC_TURN_WIDTH 4
#define SC_ALL_TURNS_WIDTH	( SC_TURN_WIDTH * NB_TURNS )


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

	wprintf( L"%lc", leftCorner );
	for ( int x = 1; x < TOTAL_WIDTH_WITH_RESULT; ++x )
	{
		if ( x % WIDTH_PER_TURN == 0 )
		{
			wprintf( L"%lc", x == TOTAL_WIDTH ? horizLineDelimWide : horizLineDelim );
			continue;
		}
		wprintf( L"%lc", horizLine );
	}
	wprintf( L"%lc", rightCorner );	
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
				wprintf( L"%lc", UTF16C_DoubleVert );
				continue;
			}
			wprintf( L"%lc", UTF16C_LightVert );
		}
	}
}


void draw_gameboard( vec2u16 upLeftPos, struct MastermindConfig const *config )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

	draw_horizontal_border( upLeftPos, true, config->nbTurns );

	upLeftPos.y += 1;
	draw_center_board( upLeftPos, config->nbCodePegPerTurn, config->nbTurns );

	upLeftPos.y += config->nbCodePegPerTurn;
	draw_horizontal_border( upLeftPos, false, config->nbTurns );
}


static void draw_peg( struct PegSlot const *slot, bool hidden )
{
	if ( slot->type == PegSlotType_EMPTY )
	{
		console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
		wprintf( L"%lc", UTF16C_SmallDottedCircle );
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
			wprintf( L"%lc", UTF16C_BigFilledCircle );
		}
	}
	else if ( slot->type == PegSlotType_KEY_PEG )
	{
		console_color_fg( slot->keyPeg ); // + 91 to match console colors, temp hack
		wprintf( L"%lc", slot->keyPeg == KeyPeg_INCORRECT ? UTF16C_SmallDottedCircle : UTF16C_SmallFilledCircle );
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
			draw_peg( &board->pegSlots[turn][y], false );
		}
	}

	// Solution
	for ( int y = 0; y < config->nbCodePegPerTurn; ++y )
	{
		console_cursor_set_position( screenPos.y + y, screenPos.x + TOTAL_WIDTH );
		draw_peg( &board->solution[y], board->hideSolution );
	}

	// Feedback
	for ( int turn = 0; turn < board->currentTurn; ++turn )
	{
		for ( int y = 0; y < ( config->nbCodePegPerTurn + 1 ) / 2; ++y )
		{
			console_cursor_set_position( screenPos.y + y + config->nbCodePegPerTurn + 1, ( screenPos.x - 1 ) + ( turn * WIDTH_PER_TURN ) );
			draw_peg( &board->pegSlots[turn][y * 2 + config->nbCodePegPerTurn], false );
			if ( y + 1 < config->nbCodePegPerTurn )
			{
				console_cursor_set_position( screenPos.y + y + config->nbCodePegPerTurn + 1, screenPos.x + ( turn * WIDTH_PER_TURN ) );
				draw_peg( &board->pegSlots[turn][y * 2 + config->nbCodePegPerTurn + 1], false );
			}
		}
	}
}


void draw_game( struct MastermindV2 *mastermind )
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
	mastermindv2_draw_selected_peg( mastermind );
}


void draw_title( vec2u16 const screenSize )
{
	short const totalEmptySpaces = screenSize.x - 56;
	short const spacesEachSide = (short)( ((float)totalEmptySpaces / 2.0f ) + 0.5f);

	console_color_fg( ConsoleColorFG_RED );

	console_cursor_set_position( 2, spacesEachSide + 1 );
	wprintf( L" __  __           _                      " );
	console_color_fg( ConsoleColorFG_GREEN );
	wprintf( L"_" );
	console_color_fg( ConsoleColorFG_RED );
	wprintf( L"           _ " );

	console_cursor_set_position( 3, spacesEachSide + 1 );
	wprintf( L"|  \\/  | __ _ ___| |_ ___ _ __ _ __ ___ " );
	console_color_fg( ConsoleColorFG_GREEN );
	wprintf( L"(_)" );
	console_color_fg( ConsoleColorFG_RED );
	wprintf( L"_ __   __| |" );

	console_cursor_set_position( 4, spacesEachSide + 1 );
	wprintf( L"| |\\/| |/ _` / __| __/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` |" );

	console_cursor_set_position( 5, spacesEachSide + 1 );
	wprintf( L"| |  | | (_| \\__ \\ ||  __/ |  | | | | | | | | | | (_| |" );

	console_cursor_set_position( 6, spacesEachSide + 1 );
	wprintf( L"|_|  |_|\\__,_|___/\\__\\___|_|  |_| |_| |_|_|_| |_|\\__,_|" );
}


int main( void )
{
	if ( !console_global_init( "Mastermind Game", true ) )
	{
		fprintf( stderr, "[FATAL ERROR]: Failed to init the console. Aborting." );
		return 1;
	}

	struct FPSCounter *const fpsCounter = fpscounter_init();
	if ( fpsCounter == NULL )
	{
		fprintf( stderr, "[FATAL ERROR]: Failed to init FPS Counter. Aborting." );
		return 2;
	}

	srand( time( NULL ) );

	struct MastermindV2 mastermind;
	mastermindv2_init( &mastermind );
	mastermindv2_start_game( &mastermind );

	HANDLE hOut = console_output_handle();
	vec2u16 newSize = console_screen_get_size( hOut );
	vec2u16 oldSize = (vec2u16) {}; // Not equal to newSize to trigger a first draw at the beginning.

	bool mainLoop = true;
	while ( mainLoop )
	{
		DWORD nbEvents = 0;
		GetNumberOfConsoleInputEvents( console_input_handle(), &nbEvents );
		while ( nbEvents-- > 0 )
		{
			DWORD cNumRead;
			INPUT_RECORD irInBuf;
			ReadConsoleInput( console_input_handle(), &irInBuf, 1, &cNumRead );

			if ( irInBuf.EventType == WINDOW_BUFFER_SIZE_EVENT )
			{
				COORD const size = irInBuf.Event.WindowBufferSizeEvent.dwSize;
				newSize = *(vec2u16 *)&size;
				continue;
			}

			if ( irInBuf.EventType == KEY_EVENT && irInBuf.Event.KeyEvent.bKeyDown )
			{
				enum KeyInput input;
				if ( !key_input_from_u32( irInBuf.Event.KeyEvent.wVirtualKeyCode, &input ) )
				{
					continue;
				}

				console_cursor_set_position( 20, 1 );
				wprintf( L"Key input: %2u", input ); 

				// TODO: mastermind_try_consume_input( )
				// Where false -> not consumed (key not used by board game)
				//       true  -> consumed
				// if false, continue to process
				// It will allow us to define keys for an associated board
				// However, we need to prevent having keys on 2 differents action

				if ( input == KeyInput_ARROW_DOWN )
				{
					mastermindv2_next_peg_in_row( &mastermind );
					draw_game( &mastermind ); // Costly operation because it redraw the entire board
				}
				else if ( input == KeyInput_ARROW_UP )
				{
					mastermindv2_previous_peg_in_row( &mastermind );
					draw_game( &mastermind ); // Costly operation because it redraw the entire board
				}
				else if ( input == KeyInput_ARROW_LEFT )
				{
					mastermind_codepeg_color_prev( &mastermind );
				}
				else if ( input == KeyInput_ARROW_RIGHT )
				{
					mastermind_codepeg_color_next( &mastermind );
				}
				else if ( input == KeyInput_ENTER )
				{
					bool const success = mastermindv2_next_turn( &mastermind );
					if ( success )
					{
						draw_game( &mastermind ); // Costly operation because it redraw the entire board
					}
				}
				else if ( input == KeyInput_ESCAPE )
				{
					mainLoop = false;
				}
				else if ( input == KeyInput_H )
				{
					mastermind.board.hideSolution = !mastermind.board.hideSolution;
					draw_game( &mastermind ); // Costly operation because it redraw the entire board
				}
				else if ( input == KeyInput_D )
				{
					mastermindv2_remove_current_codepeg( &mastermind );
					draw_game( &mastermind ); // Costly operation because it redraw the entire board
				}
				else if ( input == KeyInput_R )
				{
					mastermindv2_start_game( &mastermind );
					draw_game( &mastermind ); // Costly operation because it redraw the entire board
				}
			}
		}

		// Handle resize
		if ( newSize.x != oldSize.x || newSize.y != oldSize.y )
		{
			// Check if we need to rewrite something (x++ or y++ doesn't change anything with enough size e.g. )
			// Check if the screen is too small than required.
			console_cursor_set_position( 2, 1 );
			wprintf( L"\x1b[50M" ); // 50 is arbitrary, but it avoid cleaning up the FPS line
			// draw_title( newSize );
			draw_game( &mastermind );

			oldSize = newSize;
		}

		// End of the main loop
		nanoseconds const currFramerate = fpscounter_average_framerate( fpsCounter );
		nanoseconds const average = fpscounter_average_time( fpsCounter );

		console_cursor_set_position( 1, 1 );
		console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

		wprintf( L"%2uFPS ", currFramerate ); // %2u -> assume we can't go over 99 fps. (capped at 60fps)
		milliseconds const ms = nanoseconds_to_milliseconds( average );
		if ( ms > 999 ) wprintf( L"+" );
		wprintf( L"%3llums", ms > 999 ? 999 : ms ); // spaces at the end to remove size fluctuation if bigger size before
		wprintf( L" %ux%u", newSize.x, newSize.y ); // spaces at the end to remove size fluctuation if bigger size before
		wprintf( L"\x1b[0K");

		fpscounter_frame( fpsCounter );
	}

	fpscounter_uninit( fpsCounter );
	console_global_uninit();
	return 0;

/*	struct ConsoleScreen cscreen;
	if (! console_screen_init( &cscreen ) ) return 1; // better handling needed

	FlushConsoleInputBuffer( GetStdHandle( STD_INPUT_HANDLE ) );
	HANDLE const hStdin = GetStdHandle( STD_INPUT_HANDLE );

	while ( true )
	{
		console_screen_update_state( &cscreen );
		int newInputReceived = 0;

		DWORD nbInputs = 0;
		GetNumberOfConsoleInputEvents( hStdin, &nbInputs );
		if ( nbInputs > 0 )
		{
			enum KeyInput input;
			if ( read_next_input( hStdin, &input ) )
			{
				if ( input == KeyInput_ARROW_DOWN ) newInputReceived = 1;
				else if ( input == KeyInput_ARROW_UP )  newInputReceived = -1;
				else if ( input == KeyInput_ENTER )
				{
					if ( s_currSelect == 2 ) break; // Quit
				}
			}
		}

		if ( console_screen_is_too_small( &cscreen ) )
		{
			if ( cscreen.rewriteNeeded ) write_too_small_screen_warning( &cscreen );
		}
		else if ( cscreen.rewriteNeeded || newInputReceived != 0 )
		{
			write_hardcoded_game( &cscreen, newInputReceived );
		}

		Sleep( 32 );		
	}

	console_global_uninit();
	return 0;

	struct GameMenuList menus = {};
	game_menu_init( &menus );

	game_menu_loop( &menus );

	game_menu_shutdown( &menus );
	return 0;*/
}
