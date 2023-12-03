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


struct ContentChunk
{
	char *allocContent;
	usize capacity;
	usize currPos;
	usize nbLines; // Not necessary anymore with the new clean screen method, we can simplify the code !
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

void console_system_init( void )
{
	// Detach from the console that has launched the program to have our own console
	FreeConsole();
	AllocConsole();

	term_init();
	SetConsoleTitleA( "Mastermind Game" );
	SetConsoleCtrlHandler( ctrl_handler, TRUE );
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


static void rng_system_init()
{
	srand( time( NULL ) );
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
}


bool console_screen_is_too_small( struct ConsoleScreen *const cscreen )
{
	vec2u32 const currSize = cscreen->size[ConsoleScreenState_CURRENT];
	vec2u32 const minSize = cscreen->minSupportedSize;

	return currSize.x < minSize.x || currSize.y < minSize.y;
}


vec2u32 console_screen_get_size( struct ConsoleScreen *const cscreen )
{
	return cscreen->size[ConsoleScreenState_CURRENT];
}


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


int main( void )
{
	rng_system_init();
	console_system_init();
	signal( SIGINT, signalHandler );

	struct ConsoleScreen cscreen;
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

	return 0;

	struct GameMenuList menus = {};
	game_menu_init( &menus );

	game_menu_loop( &menus );

	game_menu_shutdown( &menus );
	return 0;
}
