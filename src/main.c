// Mastermind game in C
#include "mastermind.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main( void )
{
	// Initialize random. Not great but enough.
	srand( time( NULL ) );

	struct Mastermind game;
	bool resetSettings = true;

	for ( ;; )
	{
		if ( !mastermind_init( &game, resetSettings ) )
		{
			printf( "Failed to initialize the game. Stopping.." );
			return 1;	
		}

		mastermind_game_start( &game );
		mastermind_destroy( &game );

		// Do you want to play again ?
		// Y -> loop again + Reset Settings ?
		// N -> break loop
		break;
	}

	return 0;
}
