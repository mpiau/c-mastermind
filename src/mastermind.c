#include "mastermind.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>


#define ARR_COUNT( x ) ( sizeof( x ) / sizeof( x[0] ) )


// TODO: Rename the function, we can do better.
static char const *get_color_from_peg( u8 const peg )
{
	switch ( peg )
	{
		case 0: return "\x1b[1;31m";
		case 1: return "\x1b[1;32m";
		case 2: return "\x1b[1;33m";
		case 3: return "\x1b[1;35m";
		case 4: return "\x1b[1;36m";
		case 5: return "\x1b[1;37m";
		default: exit( 1 ); // TODO: Do something better than an exit.
	}
}


// TODO: Rename the function, color_end doesn't mean anything
static char const *get_color_end()
{
	return "\033[0m";
}


// TODO: begin with key_pegs ? Or pegs_key ?
static char const *get_key_pegs_color( enum KeyPegs const keyPegs )
{
	switch ( keyPegs )
	{
		case KEY_PEGS_NOTHING: return "\033[0m ";
		case KEY_PEGS_PARTIAL: return "\x1b[1;47m \033[0m";
		case KEY_PEGS_CORRECT: return "\x1b[1;41m \033[0m";
		default: exit( 2 ); // TODO: Do something better than an exit.
	}
}


void board_display( u8 *const board, usize const boardSize )
{
	printf( "[" );
	for ( usize i = 0; i < boardSize; ++i )
	{
		printf( " %s%u", get_color_from_peg( board[i] ), board[i] ); // Redondancy of the value board[i] here.
	}
	printf( "%s ]", get_color_end() );
}


void feedback_display( enum KeyPegs *const keyPegs, usize const boardSize )
{
	printf( "{" );
	for ( usize i = 0; i < boardSize; ++i )
	{
		printf( " %s", get_key_pegs_color( keyPegs[i] ) );
	}
	printf( " }" );
}


void board_generate( struct Mastermind *const game )
{
	size_t const codeLength = game->settings.pegsCodeLength;
	size_t const colorsNumber = game->settings.colorsNumber;
	bool const allowDuplicates = game->settings.allowDuplicatePegs;
	u8 *const code = game->codemaker;

	bool used[CODE_PEGS_LENGTH_MAX] = {};
	for ( usize i = 0; i < codeLength; ++i )
	{
		do
		{
			code[i] = rand() % colorsNumber;
		} while ( !allowDuplicates && used[code[i]] );

		used[code[i]] = true;
	}
}


bool is_valid_user_input( struct MastermindSettings const *const settings, size_t *const board, size_t const boardSize )
{
	bool used[PEGS_COLORS_NUMBER_MAX] = {};
	for ( u8 i = 0; i < boardSize; ++i )
	{
		if ( board[i] >= settings->colorsNumber )
		{
			printf( "%u is an invalid peg. Please, try again.\n", board[i] );
			return false;
		}

		if ( used[board[i]] )
		{
			printf( "All pegs need to be different. Please, try again.\n" );
			return false;
		}

		used[board[i]] = true;
	}

	return true;
}


void get_user_input( struct MastermindSettings const *const settings, u8 *const board, size_t const boardSize )
{
	while ( true )
	{
		size_t userboard[CODE_PEGS_LENGTH_MAX] = {};
		for ( size_t i = 0; i < boardSize; ++i )
		{
			// TODO: Remove that scanf, it shouldn't exist.
			printf( "Peg %u > ", i + 1 );
			scanf( "%u", &userboard[i] );
		}

		if ( !is_valid_user_input( settings, userboard, boardSize ) )
		{
			continue;
		}

		for ( size_t i = 0; i < boardSize; ++i )
		{
			board[i] = (u8)userboard[i];
		}

		return;
	}
}


void generate_feedback( struct Mastermind *const game )
{
	// Note: Investigate the behaviour when duplicates are authorized. 
	// This code assume that we don't have any duplicates, and I believe that we'll need to handle that differently.
	usize correct = 0;
	usize partial = 0;
	for ( size_t i = 0; i < game->settings.pegsCodeLength; ++i )
	{
		if ( game->codebreaker[i] == game->codemaker[i] )
		{
			correct += 1;
			continue;
		}

		// TODO: That second for-loop seems... too much ? Need a better algo, this one is easy-not-pretty
		for ( size_t j = 0; j < game->settings.pegsCodeLength; ++j )
		{
			if ( i == j ) continue;
			if ( game->codebreaker[i] == game->codemaker[j] )
			{
				partial += 1;
				break;
			}
		}
	}

	usize curPos = 0;
	while( correct-- > 0 ) { game->keyPegs[curPos++] = KEY_PEGS_CORRECT; }
	while( partial-- > 0 ) { game->keyPegs[curPos++] = KEY_PEGS_PARTIAL; }
}


static void mastermind_display_rules( struct MastermindSettings const *const settings )
{
	// TODO: We need to explain the key pegs as well ( red / white / nothing )
	printf( "- You have %u turns to guess the code.\n", settings->turnsNumber );
	if ( settings->allowDuplicatePegs )
	{
		printf( "- The code composed of %u pegs can have duplicates.\n", settings->pegsCodeLength );
	}
	else
	{
		printf( "- The code will always have %u differents pegs.\n", settings->pegsCodeLength );
	}

	printf( "- Available pegs:" );
	for ( usize i = 0; i < settings->colorsNumber; ++i )
	{
		printf( " %s%u", get_color_from_peg( i ), i );
	}
	printf( "%s\n", get_color_end() );
}


static void init_settings( struct MastermindSettings *const settings )
{
	assert( settings );

	// TODO: Ask the player instead

	settings->pegsCodeLength = 4;
	settings->colorsNumber = 6;
	settings->turnsNumber = 12;
	settings->allowDuplicatePegs = false;

	// TODO: Settings validation
}


bool mastermind_init( struct Mastermind *const mastermind, bool const resetSettings )
{
	assert( mastermind );

	if ( resetSettings )
	{
		init_settings( &mastermind->settings );
	}
	memset( mastermind->codemaker, 0, sizeof( mastermind->codemaker ) );
	memset( mastermind->codebreaker, 0, sizeof( mastermind->codebreaker ) );
	memset( mastermind->keyPegs, 0, sizeof( mastermind->keyPegs ) );
	mastermind->currentTurn = 1;
	mastermind->status = GAME_STATUS_NOT_STARTED;

	return true;
}


void mastermind_destroy( struct Mastermind *const mastermind )
{
	assert( mastermind );

	// No dynamic allocation (yet?), nothing to do.
}


void mastermind_game_start( struct Mastermind *const mastermind )
{
	mastermind->status = GAME_STATUS_ONGOING;

	struct MastermindSettings const *const settings = &mastermind->settings;

	printf( "=== MASTERMIND GAME ===\n" );
	mastermind_display_rules( settings );
	printf( "=== Good luck ! ===\n" );

	board_generate( mastermind );

	size_t const nbTurnDigits = settings->turnsNumber < 10 ? 1 : 2;
	while ( mastermind->status == GAME_STATUS_ONGOING )
	{
		printf( " ----- Turn %*u / %*u ----- \n", nbTurnDigits, mastermind->currentTurn, nbTurnDigits, settings->turnsNumber );
		get_user_input( settings, mastermind->codebreaker, settings->pegsCodeLength );
		generate_feedback( mastermind );

		printf( "Result: " );
		board_display( mastermind->codebreaker, settings->pegsCodeLength );
		printf( " - " );
		feedback_display( mastermind->keyPegs, settings->pegsCodeLength );
		printf( "\n" );

		if ( memcmp( mastermind->codemaker, mastermind->codebreaker, settings->pegsCodeLength ) == 0 ) 
		{
			mastermind->status = GAME_STATUS_VICTORY;
			continue;
		}
		else if ( mastermind->currentTurn == settings->turnsNumber )
		{
			mastermind->status = GAME_STATUS_DEFEAT;
		}

		mastermind->currentTurn++;
	}

	printf( "---------- END ----------\n" );

	if ( mastermind->status == GAME_STATUS_VICTORY )
	{
		printf( "Congratulation, you won !!\n" );
	}
	else
	{
		printf( "You ran out of attempts... The code was: " );
		board_display( mastermind->codemaker, settings->pegsCodeLength );
		printf( "\nYou will be better next time !\n" );
	}
}