// Mastermind game in C
#include "game_menus.h"
#include "mastermind.h"
#include "terminal.h"
#include "console.h"
#include "characters_list.h"
#include "core_unions.h"

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

// https://www.w3schools.com/charsets/ref_utf_box.asp 
// https://www.compart.com/en/unicode/U+25CF

#define NB_TURNS 12
#define SC_TURN_WIDTH 4
#define SC_ALL_TURNS_WIDTH	( SC_TURN_WIDTH * NB_TURNS )


void draw_horizontal_border( vec2u16 const beginCoords, bool const isTopBorder, u16 const nbTurns )
{
	utf16 const leftCorner  = isTopBorder ? UTF16C_DoubleDownRight : UTF16C_DoubleUpRight;
	utf16 const rightCorner = isTopBorder ? UTF16C_DoubleDownLeft : UTF16C_DoubleUpLeft;

	utf16 const horizLineDelim     = isTopBorder ? UTF16C_DoubleHorizSingleDown : UTF16C_DoubleHorizSingleUp;
	utf16 const horizLineDelimWide = isTopBorder ? UTF16C_DoubleHorizAndDown : UTF16C_DoubleHorizAndUp;
	utf16 const horizLine = UTF16C_DoubleHoriz;

	u16 const WIDTH_PER_TURN          = 4;
	u16 const TOTAL_WIDTH             = WIDTH_PER_TURN * nbTurns;
	u16 const TOTAL_WIDTH_WITH_RESULT = TOTAL_WIDTH + WIDTH_PER_TURN;

	console_cursor_set_position( beginCoords.y, beginCoords.x );

	wprintf( L"%lc", leftCorner );
	for ( int x = 1; x < TOTAL_WIDTH_WITH_RESULT; ++x )
	{
		if ( x % WIDTH_PER_TURN == 0 )
		{
			wprintf( L"%lc", x == TOTAL_WIDTH ? horizLineDelimWide : horizLineDelim );
			continue;
		}
		wprintf( L"%lc", horizLine );
	}
	wprintf( L"%lc", rightCorner );	
}


void draw_center_board( vec2u16 const screenPos, u16 const pegsPerRow, u16 const nbTurns )
{
	u16 const WIDTH_PER_TURN          = 4;
	u16 const TOTAL_WIDTH             = WIDTH_PER_TURN * nbTurns;
	u16 const TOTAL_WIDTH_WITH_RESULT = TOTAL_WIDTH + WIDTH_PER_TURN;

	for ( int y = 0; y < pegsPerRow; ++y )
	{
		for ( int x = 0; x <= TOTAL_WIDTH_WITH_RESULT; x += WIDTH_PER_TURN )
		{
			console_cursor_set_position( screenPos.y + y, screenPos.x + x );
			if ( x == 0 || x == TOTAL_WIDTH || x == TOTAL_WIDTH_WITH_RESULT )
			{
				wprintf( L"%lc", UTF16C_DoubleVert );
				continue;
			}
			wprintf( L"%lc", UTF16C_LightVert );
		}
	}
}


void draw_gameboard( vec2u16 upLeftPos, u16 const pegsPerRow, u16 const nbTurns )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

	draw_horizontal_border( upLeftPos, true, nbTurns );

	upLeftPos.y += 1;
	draw_center_board( upLeftPos, pegsPerRow, nbTurns );

	upLeftPos.y += pegsPerRow;
	draw_horizontal_border( upLeftPos, false, nbTurns );
}


void draw_gameboard_content( vec2u16 screenPos, u16 const pegsPerRow, u16 const nbTurns )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

	utf16 const nopeg = UTF16C_SmallDottedCircle;

	u16 const WIDTH_PER_TURN          = 4;
	u16 const TOTAL_WIDTH             = WIDTH_PER_TURN * nbTurns;
	u16 const TOTAL_WIDTH_WITH_RESULT = TOTAL_WIDTH + WIDTH_PER_TURN;

	for ( int y = 0; y < pegsPerRow; ++y )
	{
		for ( int x = 0; x < TOTAL_WIDTH_WITH_RESULT; x += WIDTH_PER_TURN )
		{
			console_cursor_set_position( screenPos.y + y, screenPos.x + x );
			wprintf( L"%lc", x == TOTAL_WIDTH ? L'?' : nopeg );
		}
	}
}


void draw_game( vec2u16 const screenSize )
{
	vec2u16 const upLeft = (vec2u16) { .x = 10, .y = 7 };
	u16 const nbTurns = 12;
	u16 const pegsPerRow = 4;

	draw_gameboard( upLeft, pegsPerRow, nbTurns );

	// Need to take actual data to set : 
	// The colored pegs / placeholder
	// ? at the end or solution if finished
	// The feedback row up to current turn (excluded)
	draw_gameboard_content( (vec2u16) { .x = upLeft.x + 2, .y = upLeft.y + 1 }, pegsPerRow, nbTurns );
}


void draw_title( vec2u16 const screenSize )
{
	short const totalEmptySpaces = screenSize.x - 56;
	short const spacesEachSide = (short)( ((float)totalEmptySpaces / 2.0f ) + 0.5f);

	console_color_fg( ConsoleColorFG_RED );

	console_cursor_set_position( 1, spacesEachSide + 1 );
	wprintf( L" __  __           _                      " );
	console_color_fg( ConsoleColorFG_GREEN );
	wprintf( L"_" );
	console_color_fg( ConsoleColorFG_RED );
	wprintf( L"           _ " );

	console_cursor_set_position( 2, spacesEachSide + 1 );
	wprintf( L"|  \\/  | __ _ ___| |_ ___ _ __ _ __ ___ " );
	console_color_fg( ConsoleColorFG_GREEN );
	wprintf( L"(_)" );
	console_color_fg( ConsoleColorFG_RED );
	wprintf( L"_ __   __| |" );

	console_cursor_set_position( 3, spacesEachSide + 1 );
	wprintf( L"| |\\/| |/ _` / __| __/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` |" );

	console_cursor_set_position( 4, spacesEachSide + 1 );
	wprintf( L"| |  | | (_| \\__ \\ ||  __/ |  | | | | | | | | | | (_| |" );

	console_cursor_set_position( 5, spacesEachSide + 1 );
	wprintf( L"|_|  |_|\\__,_|___/\\__\\___|_|  |_| |_| |_|_|_| |_|\\__,_|" );
}


u64 get_timestamp_nanoseconds()
{
    struct timespec time;
    clock_gettime( CLOCK_MONOTONIC, &time );
    return time.tv_nsec; // time.tv_sec * 1000000 + time.tv_nsec / 1000;
}


int main( void )
{
	if ( !console_global_init( "Mastermind Game", true ) )
	{
		fprintf( stderr, "[FATAL ERROR]: Failed to init the console. Aborting." );
		return 1;
	}

	srand( time( NULL ) );

	HANDLE hOut = console_output_handle();
	vec2u16 newSize = console_screen_get_size( hOut );
	vec2u16 oldSize = (vec2u16) {}; // Not equal to newSize to trigger a first draw at the beginning.
	CONSOLE_SCREEN_BUFFER_INFO csinfo;

	float const MS_TO_NANO = 1000000.0f;
	float const NANO_TO_MS = 0.000001f;
	u64 const FPS_30_NANO = (u64)roundf( ( 1000.0f * MS_TO_NANO ) / 30.0f );
	u64 nanoDeltaSamples[8] = {};
	u64 nanoDeltaSamplesTotal = 0;
	u8 nanoDeltaSampleIdx = 0;

	while ( true )
	{
		u64 const timestampStartNano = get_timestamp_nanoseconds();

		DWORD nbEvents = 0;
		GetNumberOfConsoleInputEvents( console_input_handle(), &nbEvents );
		while ( nbEvents-- > 0 )
		{
			DWORD cNumRead;
			INPUT_RECORD irInBuf;
			ReadConsoleInput( console_input_handle(), &irInBuf, 1, &cNumRead );

			if ( irInBuf.EventType == WINDOW_BUFFER_SIZE_EVENT )
			{
				COORD const size = irInBuf.Event.WindowBufferSizeEvent.dwSize;
				newSize = *(vec2u16 *)&size;
			}
		}

		// Handle resize
		if ( newSize.x != oldSize.x || newSize.y != oldSize.y )
		{
			// Check if we need to rewrite something (x++ or y++ doesn't change anything with enough size e.g. )
			// Check if the screen is too small than required.
			console_screen_clear();
			draw_title( newSize );
			draw_game( newSize );
			oldSize = newSize;
		}

		u64 timestampEndNano = get_timestamp_nanoseconds();
		u64 deltaNanoNoSleep = timestampEndNano - timestampStartNano;

		if ( deltaNanoNoSleep < FPS_30_NANO )
		{
			u64 const spinLockTime = 5 * MS_TO_NANO;
			u64 const sleepMsTime = ( FPS_30_NANO - abs( deltaNanoNoSleep - spinLockTime ) ) * NANO_TO_MS;
			console_cursor_set_position( 20, 1 );
			wprintf( L"\x1b[K Sleeping for %u ms... ( %llu )", sleepMsTime, ( FPS_30_NANO - abs( deltaNanoNoSleep - spinLockTime ) ) ); 
			Sleep( sleepMsTime );
//			Sleep( ( FPS_30_NANO - fabs( deltaNanoNoSleep - 5000.0f ) * 1000.0f ) );
		}

		do
		{
			timestampEndNano = get_timestamp_nanoseconds();
		}
		while( timestampEndNano - timestampStartNano < FPS_30_NANO );

		u64 deltaNano = timestampEndNano - timestampStartNano;
		u64 framerate = deltaNano;
		nanoDeltaSamplesTotal -= nanoDeltaSamples[nanoDeltaSampleIdx];
		nanoDeltaSamplesTotal += framerate;
		nanoDeltaSamples[nanoDeltaSampleIdx] = framerate;
		nanoDeltaSampleIdx = ( nanoDeltaSampleIdx + 1 ) % ARR_COUNT( nanoDeltaSamples );

		u64 averageNanoDelta = nanoDeltaSamplesTotal / ARR_COUNT( nanoDeltaSamples );

		u32 currentFramerate = (u32)( roundf( 1.0f / ( averageNanoDelta / ( 1000 * MS_TO_NANO ) ) ) );

		console_cursor_set_position( 21, 1 );
		wprintf( L"\x1b[K" );
		wprintf( L" %u fps (%llu ms) (before capped %llu ms)", currentFramerate, deltaNano * NANO_TO_MS, deltaNanoNoSleep * NANO_TO_MS );
		for ( int i = 0; i < 8; ++i )
		{
			console_cursor_set_position( 22 + i, 1 );
			wprintf( L"\x1b[K" );
			wprintf( L"nano time : %llu", nanoDeltaSamples[i] );
		}

		// ////////

	}

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
