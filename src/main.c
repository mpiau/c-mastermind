// Mastermind game in C
#include "mastermind.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>

#define ARR_COUNT( x ) ( sizeof( x ) / sizeof( x[0] ) )


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
		default: exit( 1 );
	}
	// static_assert( COLOR_COUNT == 6, "Update the switch case if the number of colors is updated" );
}

static inline char const *get_color_end( void )
{
	return "\033[0m";
}


void board_display( u8 *const board, usize const boardSize )
{
	printf( "[" );
	for ( usize i = 0; i < boardSize; ++i )
	{
		printf( " %s%u", get_color_from_peg( board[i] ), board[i] );
	}
	printf( "%s ]", get_color_end() );
}


void board_generate( struct Mastermind *const game )
{
	size_t const codeLength = game->settings.pegsCodeLength;
	size_t const colorsNumber = game->settings.colorsNumber;
	bool const allowDuplicates = game->settings.allowDuplicatePegs;
	u8 *const code = game->codemaker;

	bool used[PEGS_CODE_LENGTH_MAX] = {};
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
		size_t userboard[PEGS_CODE_LENGTH_MAX] = {};
		for ( size_t i = 0; i < boardSize; ++i )
		{
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


static void mastermind_display_rules( struct MastermindSettings const *const settings )
{
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


static void mastermind_main_loop( struct Mastermind *const mastermind )
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

		printf( "Result: " );
		board_display( mastermind->codebreaker, settings->pegsCodeLength );
		printf( "\n" );

		// TODO: Check the differences between the codebreaker and the codemaker
		// TODO: Store & Display the difference result

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


static void mastermind_init_settings( struct MastermindSettings *const settings )
{
	assert( settings );

	// TODO: Ask the player instead

	settings->pegsCodeLength = PEGS_CODE_LENGTH_MIN;
	settings->colorsNumber = PEGS_COLORS_NUMBER_MIN;
	settings->turnsNumber = TURNS_NUMBER_MIN;
	settings->allowDuplicatePegs = false;
}


static void mastermind_init( struct Mastermind *const game, bool const resetSettings )
{
	assert( game );

	if ( resetSettings )
	{
		mastermind_init_settings( &game->settings );
	}
	memset( game->codemaker, 0, sizeof( game->codemaker ) );
	memset( game->codebreaker, 0, sizeof( game->codebreaker ) );
	game->currentTurn = 1;
	game->status = GAME_STATUS_NOT_STARTED;
}


static void mastermind_destroy( struct Mastermind *const game )
{
	assert( game );

	// No dynamic allocation (yet?), nothing to do.
}


int main( void )
{
	// Initialize random. Not great but enough.
	srand( time( NULL ) );

	struct Mastermind game;
	mastermind_init( &game, true );

	mastermind_main_loop( &game );

	mastermind_destroy( &game );
	return 0;
}
