// Mastermind game in C
#include "mastermind.h"

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


static DWORD s_oldConsoleMode = 0;

void ResetConsoleModeOnExit( void )
{
	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	SetConsoleMode( hStdin,  s_oldConsoleMode );
}


int main( void )
{
	// Initialize random. Not great but enough.
	srand( time( NULL ) );

	signal( SIGINT, signalHandler );

	// https://stackoverflow.com/questions/51726140/console-with-enable-line-input-doesnt-pass-r
	_setmode( _fileno( stdin ), _O_BINARY );

	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	if ( !hStdin ) { return 1; }

	GetConsoleMode( hStdin, &s_oldConsoleMode );

	DWORD consoleModeIn  = s_oldConsoleMode & ~( ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT );
	if ( SetConsoleMode( hStdin,  consoleModeIn ) )
	{
		atexit( ResetConsoleModeOnExit );
	}

	DWORD cNumRead, fdwMode, i;
	INPUT_RECORD irInBuf;

	while ( ReadConsoleInput( hStdin, &irInBuf, 1, &cNumRead ) )
	{
		switch( irInBuf.EventType )
			{
			case KEY_EVENT: // keyboard input
				if ( !irInBuf.Event.KeyEvent.bKeyDown ) break;
				switch ( irInBuf.Event.KeyEvent.wVirtualKeyCode )
				{
					case VK_LEFT: printf( "\b" /*back one char in the line */ ); break;
					case VK_RIGHT: printf( " " ); break;
					default: break;
				};
				break;
			default:
				break;
		}
	}




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
