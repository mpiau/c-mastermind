//temp
// https://learn.microsoft.com/en-ca/windows/win32/api/winuser/nf-winuser-setsystemcursor?redirectedfrom=MSDN
#define OEMRESOURCE 

// Mastermind game in C
#include "game_menus.h"
#include "mastermind_v2.h"
#include "terminal.h"
#include "console.h"
#include "characters_list.h"
#include "core_unions.h"
#include "fps_counter.h"
#include "widgets/widget_timer.h"
#include "widgets/widget_fpsbar.h"
#include "widgets/widgets_utils.h"

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

// Get cursor position
// ESC [ 6 n 	DECXCPR 	Report Cursor Position 	Emit the cursor position as: ESC [ <r> ; <c> R Where <r> = cursor row and <c> = cursor column

// https://www.w3schools.com/charsets/ref_utf_box.asp 
// https://www.compart.com/en/unicode/U+25CF


void draw_title( vec2u16 const screenSize )
{
	short const totalEmptySpaces = screenSize.x - 56;
	short const spacesEachSide = (short)( ((float)totalEmptySpaces / 2.0f ) + 0.5f);

	console_color_fg( ConsoleColorFG_RED );

	console_cursor_set_position( 2, spacesEachSide + 1 );
	wprintf( L" __  __           _                      " );
	console_color_fg( ConsoleColorFG_GREEN );
	wprintf( L"_" );
	console_color_fg( ConsoleColorFG_RED );
	wprintf( L"           _ " );

	console_cursor_set_position( 3, spacesEachSide + 1 );
	wprintf( L"|  \\/  | __ _ ___| |_ ___ _ __ _ __ ___ " );
	console_color_fg( ConsoleColorFG_GREEN );
	wprintf( L"(_)" );
	console_color_fg( ConsoleColorFG_RED );
	wprintf( L"_ __   __| |" );

	console_cursor_set_position( 4, spacesEachSide + 1 );
	wprintf( L"| |\\/| |/ _` / __| __/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` |" );

	console_cursor_set_position( 5, spacesEachSide + 1 );
	wprintf( L"| |  | | (_| \\__ \\ ||  __/ |  | | | | | | | | | | (_| |" );

	console_cursor_set_position( 6, spacesEachSide + 1 );
	wprintf( L"|_|  |_|\\__,_|___/\\__\\___|_|  |_| |_| |_|_|_| |_|\\__,_|" );
}


int main( void )
{
	if ( !console_global_init( "Mastermind Game", true ) )
	{
		fprintf( stderr, "[FATAL ERROR]: Failed to init the console. Aborting." );
		return 1;
	}

	struct FPSCounter *const fpsCounter = fpscounter_init();
	if ( fpsCounter == NULL )
	{
		fprintf( stderr, "[FATAL ERROR]: Failed to init FPS Counter. Aborting." );
		return 2;
	}
	widget_fpsbar_init( fpsCounter );

	srand( time( NULL ) );

	struct MastermindV2 mastermind;
	mastermindv2_init( &mastermind );
	mastermindv2_start_game( &mastermind );

	HANDLE hOut = console_output_handle();
	vec2u16 newSize = console_screen_get_size( hOut );
	vec2u16 oldSize = (vec2u16) {}; // Not equal to newSize to trigger a first draw at the beginning.
	nanoseconds resizeTimestamp = 0;
	nanoseconds WAIT_BEFORE_RESIZE = 100 * 1000 * 1000; // 100ms

	vec2u16 mousePos = {};

/*	struct WidgetBorder border1 = {};
	border1.upLeft = (screenpos) { .x = 30, .y = 10 };
	border1.size = (vec2u16) { .x = 60, .y = 10 };
	border1.optTitle = L"Test";
*/
	struct WidgetBorder border2 = {};
	border2.upLeft = (screenpos) { .x = 1, .y = 1 };
	border2.size = (vec2u16) { .x = 120, .y = 30 };
	border2.optTitle = L"Mastermind";

	struct WidgetBorder border3 = {};
	border3.upLeft = (screenpos) { .x = 2, .y = 3 };
	border3.size = (vec2u16) { .x = 30, .y = 20 };
	border3.optTitle = L"Rules";

	struct WidgetBorder border4 = {};
	border4.upLeft = (screenpos) { .x = 32, .y = 3 };
	border4.size = (vec2u16) { .x = 30, .y = 27 };
	border4.optTitle = L"Board";

	struct WidgetTimer timer = {};
	timer.screenData.upLeft = (vec2u16){ .x = newSize.x - 18, .y = 18 };
	timer.screenData.bottomRight = (vec2u16){ .x = newSize.x, .y = 20 };
	timer.screenData.name = NULL;//L"Countdown";
	timer.lastUpdateTimestamp = get_timestamp_nanoseconds() / NANOSECONDS;
	timer.totalDuration = 60;
	timer.endTimerTimestamp = timer.lastUpdateTimestamp + timer.totalDuration;

	bool mainLoop = true;
	while ( mainLoop )
	{
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
				resizeTimestamp = get_timestamp_nanoseconds();
				continue;
			}
			else if ( irInBuf.EventType == MOUSE_EVENT )
			{
				// Can't change the mouse icon in console app =( We will need to change some things
				// To show to the user that the mouse is still usable
				mousePos = *(vec2u16 *)&irInBuf.Event.MouseEvent.dwMousePosition;
				continue;
			}
			else if ( irInBuf.EventType == KEY_EVENT && irInBuf.Event.KeyEvent.bKeyDown )
			{
				enum KeyInput input;
				if ( !key_input_from_u32( irInBuf.Event.KeyEvent.wVirtualKeyCode, &input ) )
				{
					continue;
				}

				console_cursor_set_position( 20, 1 );
				console_draw( L"Key input: %2u", input );

				if ( mastermind_try_consume_input( &mastermind, input ) )
				{
					continue;
				}

				if ( input == KeyInput_ESCAPE )
				{
					mainLoop = false;
				}
			}
		}

		// Handle resize
		if ( newSize.x != oldSize.x || newSize.y != oldSize.y )
		{
			// On resize : wait Xms before redraw
			// If another resize occurs during that delay, reset and wait again.
			// This way, we save plenty of redraw frames while the user messes up with his window size
			nanoseconds timestamp = get_timestamp_nanoseconds();
			if ( timestamp - resizeTimestamp >= WAIT_BEFORE_RESIZE )
			{
				// Check if we need to rewrite something (x++ or y++ doesn't change anything with enough size e.g. )
				// Check if the screen is too small than required.
//				console_cursor_set_position( 2, 1 );
//				console_draw( L"\x1b[50M" ); // 50 is arbitrary, but it avoid cleaning up the FPS line

				// !! draw_entire_game( &mastermind, newSize );

				oldSize = newSize;

				// !! widget_timer_redraw( &timer );

//				widget_utils_draw_borders( &border1, newSize );
				widget_utils_draw_borders( &border2, newSize );
				widget_utils_draw_borders( &border3, newSize );
				widget_utils_draw_borders( &border4, newSize );

				enum ConsoleColorFG colors[] = {
					ConsoleColorFG_GREEN,
					ConsoleColorFG_RED,
					ConsoleColorFG_CYAN,
					ConsoleColorFG_MAGENTA,
					ConsoleColorFG_YELLOW,
					ConsoleColorFG_WHITE,
					ConsoleColorFG_BLUE
				};

				for ( int i = 0; i < ARR_COUNT( colors ); ++i )
				{
					int x = 10 + i * 8;
					console_color_fg( colors[i] );
					console_cursor_set_position( 10, x );
					console_draw( L",d88b." );
					console_cursor_set_position( 11, x );
					console_draw( L"888888");
					console_cursor_set_position( 12, x );
					console_draw( L"`Y88P'" );
				}
				console_color_reset();
			}
		}

		// End of the main loop

		console_cursor_set_position( 2, 15 );
		console_draw( L" Screen: %ux%u | Mouse: %ux%u  ", newSize.x, newSize.y, mousePos.x, mousePos.y );

		// !! widget_timer_update( &timer, false );
		widgets_frame();

		console_refresh();

		fpscounter_frame( fpsCounter );
	}

	widget_fpsbar_uninit(); // Could be done in fpsCounter directly ?
	fpscounter_uninit( fpsCounter );
	console_global_uninit();
	return 0;
}
