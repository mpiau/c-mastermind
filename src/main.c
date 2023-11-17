// Mastermind game in C
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


enum
{
	MAIN_MENU_MAX_CHOICES  = 3,
};

struct GameMainMenu
{
	u8 currentChoice;
	u8 oldChoice; // Could be a choice[2] instead but later.
	char const *menuLines[MAIN_MENU_MAX_CHOICES];
	char selectedPrefixChar;
	char defaultPrefixChar;
	enum TermColor selectedLineColor;
	enum TermColor defaultLineColor;
};


void setup_main_menu( struct GameMainMenu *const mainMenu )
{
	mainMenu->currentChoice = 0;
	mainMenu->oldChoice = 0;
	mainMenu->menuLines[0] = "Single-player";
	mainMenu->menuLines[1] = "Local multiplayer (turn-based)";
	mainMenu->menuLines[2] = "Quit";

	mainMenu->selectedPrefixChar = '>';
	mainMenu->defaultPrefixChar = ' ';

	mainMenu->selectedLineColor = TERM_BOLD_GREEN;
	mainMenu->defaultLineColor = TERM_DEFAULT;
}


void print_main_menu( struct GameMainMenu const *const mainMenu )
{
	for ( u8 i = 0; i < ARR_COUNT( mainMenu->menuLines ); ++i )
	{
		bool const isSelected = ( i == mainMenu->currentChoice );
		printf( "%s%c %s\n%s",
			isSelected ? S_COLOR_STR[mainMenu->selectedLineColor] : S_COLOR_STR[mainMenu->defaultLineColor],
			isSelected ? mainMenu->selectedPrefixChar : mainMenu->defaultPrefixChar,
			mainMenu->menuLines[i],
			S_COLOR_STR_RESET
		);
	}
}


void clear_main_menu( struct GameMainMenu const *const mainMenu )
{
	for ( u8 i = 0; i < ARR_COUNT( mainMenu->menuLines ); ++i )
	{
		printf( "\033[1A\r\033[J" );
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

static void enter_main_menu( struct GameMainMenu *const mainMenu )
{
	print_main_menu( mainMenu );

	DWORD cNumRead;
	INPUT_RECORD irInBuf;
	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	bool stayInMenu = true;
	while ( stayInMenu && ReadConsoleInput( hStdin, &irInBuf, 1, &cNumRead ) )
	{
		if ( mainMenu->currentChoice != mainMenu->oldChoice )
		{
			print_main_menu( mainMenu );
			mainMenu->oldChoice = mainMenu->currentChoice;
		}

		if ( irInBuf.EventType != KEY_EVENT ) continue;
		if ( !irInBuf.Event.KeyEvent.bKeyDown ) continue;

		if ( irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_UP && mainMenu->currentChoice > 0 )
		{
			mainMenu->currentChoice -= 1;
			clear_main_menu( mainMenu );
		}
		else if ( irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_DOWN && mainMenu->currentChoice < MAIN_MENU_MAX_CHOICES - 1 )
		{
			mainMenu->currentChoice += 1;
			clear_main_menu( mainMenu );
		}
		else if ( irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_RETURN )
		{
			stayInMenu = false;
//			clear_main_menu( mainMenu ); // Do we keep that ?
		}
	}

	if ( mainMenu->currentChoice == 0 ) mastermind_singleplayer();
}


int main( void )
{
	// Initialize random. Not great but enough.
	srand( time( NULL ) );

	signal( SIGINT, signalHandler );

	term_init();

	print_game_name();

	struct GameMainMenu mainMenu = {};
	setup_main_menu( &mainMenu );
	enter_main_menu( &mainMenu );

	return 0;
}
