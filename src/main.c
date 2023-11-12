// Mastermind game in C
#include "core_types.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ARR_COUNT( x ) ( sizeof( x ) / sizeof( x[0] ) )

enum // constants
{
	CODE_PEGS = 4,
	COLOR_COUNT = 6,
	MAX_ALLOWED_TURNS = 12
};

static_assert( MAX_ALLOWED_TURNS % 2 == 0, "MAX_ALLOWED_TURNS must be even" );
static_assert( MAX_ALLOWED_TURNS > 0, "MAX_ALLOWED_TURNS can't be inferior/equals to 0" );

struct Mastermind
{
	u8 codemaker[CODE_PEGS];   // 0 -> 5
	u8 codebreaker[CODE_PEGS]; // 0 -> 5
	u8 maxNbTurns; // Must be even according to the rules.
	u8 curTurn;
};

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
	static_assert( COLOR_COUNT == 6, "Update the switch case if the number of colors is updated" );
}

static inline char const *get_color_end( void )
{
	return "\033[0m";
}


void board_display( u8 board[static CODE_PEGS] )
{
	static_assert( CODE_PEGS == 4, "This code is made specifically for CODE_PEGS == 4" );
	printf( "[ %s%u %s%u %s%u %s%u%s ]",
		get_color_from_peg( board[0] ), board[0],
		get_color_from_peg( board[1] ), board[1],
		get_color_from_peg( board[2] ), board[2],
		get_color_from_peg( board[3] ), board[3],
		get_color_end()
	);
}


void board_generate( u8 *const board, size_t const boardSize )
{
	bool used[COLOR_COUNT] = {};
	for ( u8 i = 0; i < boardSize; ++i )
	{
		do
		{
			board[i] = rand() % COLOR_COUNT;
		} while ( used[board[i]] );

		used[board[i]] = true;
	}
}


bool is_valid_user_input( size_t *const board, size_t const boardSize )
{
	bool used[COLOR_COUNT] = {};
	for ( u8 i = 0; i < boardSize; ++i )
	{
		if ( board[i] >= COLOR_COUNT )
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


void get_user_input( u8 *const board, size_t const boardSize )
{
	assert( boardSize == CODE_PEGS );

	while ( true )
	{
		size_t userboard[CODE_PEGS] = {};
		printf( "Your attempt: " );
		if ( scanf( "%u %u %u %u", &userboard[0], &userboard[1], &userboard[2], &userboard[3] ) != 4 )
		{
			printf( "4 inputs are needed. Example: \'4 2 1 3\'. Please, try again.\n" );
			continue;
		}

		if ( !is_valid_user_input( userboard, CODE_PEGS ) )
		{
			continue;
		}

		board[0] = (u8)userboard[0];
		board[1] = (u8)userboard[1];
		board[2] = (u8)userboard[2];
		board[3] = (u8)userboard[3];
		break;
	}
}


int main( void )
{
	srand( time( NULL ) );

	printf( "=== MASTERMIND GAME ===\n" );
	printf( "- You have 12 turns to guess the code.\n" );
	printf( "- The code will always have 4 differents pegs.\n" );
	printf( "- Available pegs: %s%u %s%u %s%u %s%u %s%u %s%u%s\n",
		get_color_from_peg( 0 ), 0,
		get_color_from_peg( 1 ), 1,
		get_color_from_peg( 2 ), 2,
		get_color_from_peg( 3 ), 3,
		get_color_from_peg( 4 ), 4,
		get_color_from_peg( 5 ), 5,
		get_color_end()
	);
	printf( "=== Good luck ! ===\n" );

	struct Mastermind gameData = {};
	board_generate( gameData.codemaker, ARR_COUNT( gameData.codemaker ) );
	gameData.maxNbTurns = MAX_ALLOWED_TURNS;
	gameData.curTurn = 1;

	while ( gameData.curTurn <= gameData.maxNbTurns )
	{
		get_user_input( gameData.codebreaker, ARR_COUNT( gameData.codebreaker ) );
		printf( "Turn %2u/%2u - ", gameData.curTurn, gameData.maxNbTurns );
		board_display( gameData.codebreaker );
		printf( "\n" );

		// TODO: Check the differences between the codebreaker and the codemaker
		// TODO: Store & Display the difference result
		// TODO: Stop the game if won.

		gameData.curTurn++;
	}

	// TODO: Check if win/lose game

	printf( "---------- END ----------\nThe code was: " );
	board_display( gameData.codemaker );

	return 0;
}
