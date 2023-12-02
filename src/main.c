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
#include <math.h>

#include <windows.h>


static void signalHandler( int const signal )
{
	if ( signal == SIGINT )
	{
		exit( 0 );
	}
}


char const *get_game_name( void )
{
	 // I used https://edukits.co/text-art/ for the ASCII generation.
	 // I needed to add some backslashes afterwards to not break the output with printf.

	return
		"      __  __           _                      _           _      \n"
		"     |  \\/  | __ _ ___| |_ ___ _ __ _ __ ___ (_)_ __   __| |     \n"
		"     | |\\/| |/ _` / __| __/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` |     \n"
		"     | |  | | (_| \\__ \\ ||  __/ |  | | | | | | | | | | (_| |     \n"
		"     |_|  |_|\\__,_|___/\\__\\___|_|  |_| |_| |_|_|_| |_|\\__,_|     \n"
		"\n";	
}


struct ContentChunk
{
	char *allocContent;
	usize capacity;
	usize currPos;
	usize nbLines;
};


bool content_chunk_create( struct ContentChunk *const chunk, usize const capacity )
{
	assert( capacity > 0 );

	usize const typeSize = sizeof( *chunk->allocContent );

	*chunk = (struct ContentChunk ) {
		.allocContent = malloc( capacity * typeSize ),
		.capacity = capacity,
	};

	return chunk->allocContent != NULL;
}


void content_chunk_append( struct ContentChunk *const chunk, char const *format, ... )
{
	usize oldPos = chunk->currPos;

	va_list args;
	va_start ( args, format );
	chunk->currPos += vsnprintf( chunk->allocContent + chunk->currPos, chunk->capacity - chunk->currPos, format, args );
	va_end ( args );

	while ( oldPos < chunk->currPos )
	{
		if ( chunk->allocContent[oldPos++] == '\n' ) chunk->nbLines++;
	}
}


void content_chunk_destroy( struct ContentChunk *const chunk )
{
	if ( chunk->allocContent )
	{
		free( chunk->allocContent );
	}

	*chunk = (struct ContentChunk) {};
}


BOOL ctrl_handler( DWORD const ctrlType )
{
    /*switch ( ctrlType )
    {
	case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
	default: 	assert( false ); // Shouldn't happen
    }*/

	// Call cleanup code.

	exit( 0 );
}


// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences Useful
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797 

void console_init( void )
{
	// Detach from the console that has launched the program to have our own console
	FreeConsole();
	AllocConsole();

	term_init();
	SetConsoleTitleA( "Mastermind Game" );
	SetConsoleCtrlHandler( ctrl_handler, TRUE );
	ShowScrollBar( GetConsoleWindow(), SB_VERT, 0 );
}


typedef union vec2u32
{
	struct { u32 x; u32 y; };
	u32 data[2];
} vec2u32;


bool vec2u32_equals( vec2u32 const lhs, vec2u32 const rhs )
{
	return lhs.x == rhs.x && rhs.y == lhs.y;
}


bool is_screen_too_small( vec2u32 const screenSize )
{
	if ( screenSize.x < 65 || screenSize.y < 20 )
	{
		return true;
	}

	return false;
}


int main( void )
{
	// Initialize random. Not great but enough.
	srand( time( NULL ) );
	console_init();
	signal( SIGINT, signalHandler );

	struct ContentChunk gameTitleChunk;
	content_chunk_create( &gameTitleChunk, 512 );
	content_chunk_append( &gameTitleChunk, "%s%s%s", S_COLOR_STR[TERM_BOLD_YELLOW], get_game_name(), S_COLOR_STR_RESET );

	vec2u32 screenSizeOld = (vec2u32) { .x = 0, .y = 0 };
	vec2u32 screenSize = screenSizeOld;

	CONSOLE_SCREEN_BUFFER_INFO info = {};
	HANDLE stdOut = GetStdHandle( STD_OUTPUT_HANDLE );
	usize prevDisp = 0;

	bool titleDisplayed = false;

	while ( true )
	{
		GetConsoleScreenBufferInfo( stdOut, &info );
		screenSize = (vec2u32) { .x = info.dwSize.X, .y = info.dwSize.Y };

		if ( !vec2u32_equals( screenSize, screenSizeOld ) )
		{
			if ( screenSize.y != screenSizeOld.y )
			{
				titleDisplayed = false;
			}
		}

		if ( is_screen_too_small( screenSize ) )
		{
			if ( !vec2u32_equals( screenSize, screenSizeOld ) )
			{
				// H -> cursor top, 2J == clean current screen and 3J == clean scroll history
				printf( "\033[H\x1B[2J\x1B[3JAt least 100x20 is required.\nCurrent: X = %i / Y = %i\n",  screenSize.x, screenSize.y );
				prevDisp = 2;
				titleDisplayed = false;
			}
		}
		else
		{
			if ( !titleDisplayed )
			{
				printf( "\033[H\x1B[2J\x1B[3J%s", gameTitleChunk.allocContent );
				prevDisp = gameTitleChunk.nbLines;
				titleDisplayed = true;
			}
		}

		screenSizeOld = screenSize;
		Sleep( 32 );		
	}
	return 0;

	struct GameMenuList menus = {};
	game_menu_init( &menus );

	game_menu_loop( &menus );

	game_menu_shutdown( &menus );
	return 0;
}
