// Mastermind game in C
#include "game_menus.h"
#include "mastermind.h"
#include "terminal.h"

#include <fcntl.h>
#include <io.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>


static void signalHandler( int const signal )
{
	if ( signal == SIGINT )
	{
		printf( "\nCtrl+C detected, stopping the game...\n" );
		exit( 0 );
	}
}


void print_game_name( void )
{
	 // I used https://edukits.co/text-art/ for the ASCII generation.
	 // I needed to add some backslashes afterwards to not break the output with printf.

	term_print( TERM_BOLD_YELLOW,
		"      __  __           _                      _           _ \n"
		"     |  \\/  | __ _ ___| |_ ___ _ __ _ __ ___ (_)_ __   __| |\n"
		"     | |\\/| |/ _` / __| __/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` |\n"
		"     | |  | | (_| \\__ \\ ||  __/ |  | | | | | | | | | | (_| |\n"
		"     |_|  |_|\\__,_|___/\\__\\___|_|  |_| |_| |_|_|_| |_|\\__,_|\n"
		"\n"
	);
}


static void mastermind_singleplayer()
{
	struct Mastermind game;
	bool resetSettings = true;

	for ( ;; )
	{
		if ( !mastermind_init( &game, resetSettings ) )
		{
			printf( "Failed to initialize the game. Stopping.." );
			exit( 1 );
		}

		mastermind_game_start( &game );
		mastermind_destroy( &game );

		// Do you want to play again ?
		// Y -> loop again + Reset Settings ?
		// N -> break loop
		break;
	}
}


int main( void )
{
	// Initialize random. Not great but enough.
	srand( time( NULL ) );

	signal( SIGINT, signalHandler );

	term_init();

	print_game_name();

	struct GameMenuList menus = {};
	game_menu_init( &menus );

	game_menu_loop( &menus );

	game_menu_shutdown( &menus );
	return 0;
}
