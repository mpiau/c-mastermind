// Mastermind game in C
#include "game_menus.h"
#include "mastermind.h"
#include "terminal.h"
#include "console.h"

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


// System headers
#include <windows.h>

// Standard library C-style
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>

#define ESC "\x1b"
#define CSI "\x1b["

void PrintVerticalBorder()
{
	console_line_drawing_mode_enter();
	console_color( ConsoleColorFG_BRIGHT_YELLOW, ConsoleColorBG_BRIGHT_BLUE );
    printf( "%c", ConsoleLineDrawing_VERT_LINE );
	console_color_reset();
	console_line_drawing_mode_exit();
}


void PrintHorizontalBorder(COORD const Size, bool fIsTop)
{
    console_line_drawing_mode_enter();
    printf(CSI "104;93m"); // Make the border bright yellow on bright blue
    printf(fIsTop ? "l" : "m"); // print left corner 

    for (int i = 1; i < Size.X - 1; i++)
        printf("q"); // in line drawing mode, \x71 -> \u2500 "HORIZONTAL SCAN LINE-5"

    printf(fIsTop ? "k" : "j"); // print right corner
	console_color_reset();
	console_line_drawing_mode_exit();
}

void PrintStatusLine(const char* const pszMessage, COORD const Size)
{
    printf(CSI "%d;1H", Size.Y);
    printf(CSI "K"); // clear the line
    printf(pszMessage);
}

int wmain2()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        printf("Couldn't get the console handle. Quitting.\n");
        return -1;
    }

    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
    GetConsoleScreenBufferInfo(hOut, &ScreenBufferInfo);
    COORD Size;
    Size.X = ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
    Size.Y = ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;

	for ( int i = 0; i < 50; i++ ) {
		// Clear screen, tab stops, set, stop at columns 16, 32
		printf(CSI "1;1H");
		printf(CSI "2J"); // Clear screen

		int iNumTabStops = 4; // (0, 20, 40, width)
		printf(CSI "3g"); // clear all tab stops
		printf(CSI "1;20H"); // Move to column 20
		printf(ESC "H"); // set a tab stop

		printf(CSI "1;40H"); // Move to column 40
		printf(ESC "H"); // set a tab stop

		// Set scrolling margins to 3, h-2
		printf(CSI "3;%dr", Size.Y - 2);
		int iNumLines = Size.Y - 4;

		printf(CSI "1;1H");
		printf(CSI "102;30m");
		printf("Windows 10 Anniversary Update - VT Example");
		printf(CSI "0m");

		// Print a top border - Yellow
		printf(CSI "2;1H");
		PrintHorizontalBorder(Size, true);

		// // Print a bottom border
		printf(CSI "%d;1H", Size.Y - 1);
		PrintHorizontalBorder(Size, false);

		wchar_t wch;

		// draw columns
		printf(CSI "3;1H");
		int line = 0;
		for (line = 0; line < iNumLines * iNumTabStops; line++)
		{
			PrintVerticalBorder();
			if (line + 1 != iNumLines * iNumTabStops) // don't advance to next line if this is the last line
				printf("\t"); // advance to next tab stop

		}

		PrintStatusLine("Press any key to see text printed between tab stops.", Size);
		Sleep( 1000 );
	//    wch = _getwch();

		// Fill columns with output
		printf(CSI "3;1H");
		for (line = 0; line < iNumLines; line++)
		{
			int tab = 0;
			for (tab = 0; tab < iNumTabStops - 1; tab++)
			{
				PrintVerticalBorder();
				printf("line=%d", line);
				printf("\t"); // advance to next tab stop
			}
			PrintVerticalBorder();// print border at right side
			if (line + 1 != iNumLines)
				printf("\t"); // advance to next tab stop, (on the next line)
		}

		PrintStatusLine("Press any key to demonstrate scroll margins", Size);
		Sleep( 1000 );
	//    wch = _getwch();

		printf(CSI "3;1H");
		for (line = 0; line < iNumLines * 2; line++)
		{
			printf(CSI "K"); // clear the line
			int tab = 0;
			for (tab = 0; tab < iNumTabStops - 1; tab++)
			{
				PrintVerticalBorder();
				printf("line=%d", line);
				printf("\t"); // advance to next tab stop
			}
			PrintVerticalBorder(); // print border at right side
			if (line + 1 != iNumLines * 2)
			{
				printf("\n"); //Advance to next line. If we're at the bottom of the margins, the text will scroll.
				printf("\r"); //return to first col in buffer
			}
		}

		PrintStatusLine("Press any key to exit", Size);
		Sleep( 1000 );
	//    wch = _getwch();
	}
}


// Get cursor position
// ESC [ 6 n 	DECXCPR 	Report Cursor Position 	Emit the cursor position as: ESC [ <r> ; <c> R Where <r> = cursor row and <c> = cursor column

void draw_title( COORD const screenSize )
{
	wchar_t upLeft = L'╔';
	wchar_t upRight = L'╗';
	wchar_t downLeft = L'╚';
	wchar_t downRight = L'╝';
	wchar_t vertline = L'║';
	wchar_t horizline = L'═';
	wchar_t horizLineDown = L'╤';
	wchar_t horizLineUp = L'╧';
	wchar_t peg = L'⬤';
	wchar_t nopeg = L'◌';
	wchar_t noresult = L'◌';
	wchar_t result = L'●'; // ○
	wchar_t vertlinesmall = L'│';

#define NB_TURNS 12

	short currTurn = 5;

	wchar_t pegs[NB_TURNS][4];
	wchar_t turnRes[NB_TURNS][4];
	for (int i = 0; i < ARR_COUNT( pegs ); ++i )
	{
		wchar_t l = i < currTurn ? peg : L'◌';
		wchar_t e = i % 2 == 0 ? L'◌' : result;
		pegs[i][0] = l;
		pegs[i][1] = l;
		pegs[i][2] = l;
		pegs[i][3] = l;
		turnRes[i][0] = e;
		turnRes[i][1] = e;
		turnRes[i][2] = e;
		turnRes[i][3] = e;
	}

	// Step 1 : Draw gameboard
	// First line
	console_cursor_set_position( 1, 1 );
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
	wprintf( L"%lc", upLeft );
	for ( int x = 1; x < 48; ++x )
	{
		if ( x % 4 == 0 ) wprintf( L"%lc", horizLineDown );
		else wprintf( L"%lc", horizline );
	}
	wprintf( L"%lc", upRight );

	// Content
	for ( int y = 2; y < 6; ++y )
	{
		console_cursor_set_position( y, 1 );
		wprintf( L"%lc", vertline );
		for ( int x = 1; x < 48; ++x )
		{
			if ( x % 4 == 0 )
			{
				wprintf( L"%lc", vertlinesmall );
			}
			else if ( x >= 2 && (x - 2) % 4 == 0)
			{
				int curr = ( x - 2 ) / 4;
				wchar_t currPeg = pegs[curr][y - 2];
				if ( currTurn == curr )
				{
					if ( currPeg == peg )
					{
						console_color_fg( ConsoleColorFG_BRIGHT_GREEN );
					}
					else
					{
						console_color_fg( ConsoleColorFG_BRIGHT_WHITE );
					}
				}
				else
				{
					console_color_fg( ConsoleColorFG_BRIGHT_BLACK );
				}

				wprintf( L"%lc", currPeg );
				console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
			}
			else wprintf( L"%lc", L' ' );
		}
		wprintf( L"%lc", vertline );
	}

	// Last gameboard line 
	console_cursor_set_position( 6, 1 );
	wprintf( L"%lc", downLeft );
	for ( int x = 1; x < 48; ++x )
	{
		if ( x % 4 == 0 ) wprintf( L"%lc", horizLineUp );
		else wprintf( L"%lc", horizline );
	}
	wprintf( L"%lc", downRight );

	// Result lines
	for ( int y = 7; y < 9; ++y )
	{
		for ( int turn = 0; turn < NB_TURNS && turn < currTurn; ++turn )
		{
			console_cursor_set_position( y, turn * 4 + 2 );
			short idx = ( y - 7 ) * 2;
			wchar_t first = turnRes[turn][idx];
			wchar_t second = turnRes[turn][idx + 1];

			enum ConsoleColorFG color1 = first == result ? ConsoleColorFG_RED : ConsoleColorFG_WHITE;
			enum ConsoleColorFG color2 = second == result ? ConsoleColorFG_RED : ConsoleColorFG_WHITE;

			console_color_fg( color1 );
			wprintf( L"%lc", first );
			console_color_fg( color2 );
			wprintf( L"%lc", second );
		}		
	}

	console_color_reset();
	return;

		wchar_t *bbb =
		L"            Player 1\n"
		 "╔═══╤═══╤═══╤═══╤═══╤═══╤═══════╗\n"
		 "║ \x1b[1;92m⬤\x1b[0;0m │ ⬤ ╎ ◌ ╎ ◌ ║ ○ ║ ○ ║ ○ ║ ? ║\n"
		 "║ \x1b[1;91m⬤\x1b[0;0m │ ◌ ╎ ◌ ╎ ◌ ║ ○ ║ ○ ║ ○ ║ ? ║\n"
		 "║ \x1b[1;94m⬤\x1b[0;0m │ ⬤ ╎ ◌ ╎ ◌ ║ ○ ║ ○ ║ ○ ║ ? ║\n"
		 "║ \x1b[1;95m⬤\x1b[0;0m │ ◌ ╎ ◌ ╎ ◌ ║ ○ ║ ○ ║ ○ ║ ? ║\n"
		 "╚═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╝\n"
		 " ◌◌  ◌◌  ◌◌  ◌◌  ◌◌  ◌◌  ◌◌      \n"
		 " ◌◌  ◌◌  ◌◌  ◌◌  ◌◌  ◌◌  ◌◌      \n";

	wprintf( bbb );
	return;

	char title[5][128] = {};
	short titleSize = 0;

	if ( title[0][0] == '\0' )
	{
		snprintf( title[0], ARR_COUNT( title[0] ), "%s __  __           _                      %s_%s           _ ",
			S_COLOR_STR[TERM_BOLD_RED], S_COLOR_STR[TERM_BOLD_GREEN], S_COLOR_STR[TERM_BOLD_RED] );
		snprintf( title[1], ARR_COUNT( title[1] ), "|  \\/  | __ _ ___| |_ ___ _ __ _ __ ___ %s(_)%s_ __   __| |",
			S_COLOR_STR[TERM_BOLD_GREEN], S_COLOR_STR[TERM_BOLD_RED] );
		snprintf( title[2], ARR_COUNT( title[2] ), "| |\\/| |/ _` / __| __/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` |" );
		snprintf( title[3], ARR_COUNT( title[4] ), "| |  | | (_| \\__ \\ ||  __/ |  | | | | | | | | | | (_| |" );
		snprintf( title[4], ARR_COUNT( title[5] ), "|_|  |_|\\__,_|___/\\__\\___|_|  |_| |_| |_|_|_| |_|\\__,_|" );
		titleSize = strlen( title[4] );
	}

	short const totalEmptySpaces = screenSize.X - titleSize;
	short const spacesEachSide = totalEmptySpaces / 2;

	for ( int i = 0; i < ARR_COUNT( title ); ++i )
	{
		console_cursor_set_position( i + 1, spacesEachSide + 1 );
		printf( "%s", title[i] );
	}
}

#include <locale.h>

int main( void )
{
	if ( !console_global_init( "Mastermind Game", true ) )
	{
		fprintf( stderr, "[FATAL ERROR]: Failed to init the console. Aborting." );
		return 1;
	}

	srand( time( NULL ) );

	HANDLE hOut = console_output_handle();
	COORD newSize = console_screen_get_size( hOut );
	COORD oldSize = (COORD) {};
	CONSOLE_SCREEN_BUFFER_INFO csinfo;

	while ( true )
	{
		DWORD nbEvents = 0;
		GetNumberOfConsoleInputEvents( console_input_handle(), &nbEvents );
		while ( nbEvents > 0 )
		{
			DWORD cNumRead;
			INPUT_RECORD irInBuf;
			ReadConsoleInput( console_input_handle(), &irInBuf, 1, &cNumRead );

			if ( irInBuf.EventType == WINDOW_BUFFER_SIZE_EVENT )
			{
				newSize = irInBuf.Event.WindowBufferSizeEvent.dwSize;
				if ( newSize.X != oldSize.X || newSize.Y != oldSize.Y )
				{
					console_screen_clear();
					draw_title( newSize );
					oldSize = newSize;
				}
			}
		}
/*		GetConsoleScreenBufferInfo( hOut, &csinfo );
		newSize = (COORD) { .X = csinfo.dwSize.X, .Y = csinfo.dwSize.Y };*/
		Sleep( 32 );
	}


/*	COORD const size = console_screen_get_size( console_output_handle() );
	console_line_drawing_mode_enter();

	console_cursor_set_position( 1, 1 );
	printf( "%c", ConsoleLineDrawing_UPPER_LEFT );
	for ( int i = 1; i < size.X - 1; i++ )
	{
		printf( "%c", ConsoleLineDrawing_HORIZ_LINE );
	}
	printf( "%c", ConsoleLineDrawing_UPPER_RIGHT );

	for ( int i = 2; i < size.Y; i++ )
	{
		console_cursor_set_position( i, 1 );
		printf( "%c", ConsoleLineDrawing_VERT_LINE );
		console_cursor_set_position( i, size.X );
		printf( "%c", ConsoleLineDrawing_VERT_LINE );
	}

	console_cursor_set_position(size.Y, 1 );
	printf( "%c", ConsoleLineDrawing_LOWER_LEFT );
	for ( int i = 1; i < size.X - 1; i++ )
	{
		printf( "%c", ConsoleLineDrawing_HORIZ_LINE );
	}
	printf( "%c", ConsoleLineDrawing_LOWER_RIGHT );

	console_line_drawing_mode_exit();	*/
	wint_t x = _getwch();

	console_global_uninit();
	return 0;





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

	console_global_uninit();
	return 0;

	struct GameMenuList menus = {};
	game_menu_init( &menus );

	game_menu_loop( &menus );

	game_menu_shutdown( &menus );
	return 0;
}
