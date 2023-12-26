// Mastermind game in C
#include "game_menus.h"
#include "mastermind_v2.h"
#include "terminal.h"
#include "console.h"
#include "console_screen.h"
#include "characters_list.h"
#include "core_unions.h"
#include "fps_counter.h"
#include "widgets/widget_timer.h"
#include "widgets/widget_framerate.h"
#include "widgets/widget_countdown.h"
#include "widgets/widget_utils.h"
#include "mouse.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <windows.h>

/*
	// PEGS DISPLAY 
	console_draw( L",d88b." );
	console_draw( L"888888");
	console_draw( L"`Y88P'" );

	console_draw( L",db." );
	console_draw( L"`YP'" );

	console_draw( L",-." );
	console_draw( L"`-'" );

	console_draw( L" __ " );
	console_draw( L",  ." );
	console_draw( L"`  '" );
	console_draw( L" -- " );
*/

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

/*
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
*/

// TEMP
static bool s_mainLoop = true;
static struct MastermindV2 s_mastermind = {};


static void consume_input( INPUT_RECORD const *const recordedInput )
{
	assert( recordedInput );

	switch ( recordedInput->EventType )
	{
		case WINDOW_BUFFER_SIZE_EVENT:
		{
			COORD const size = recordedInput->Event.WindowBufferSizeEvent.dwSize;
			console_screen_resize( *(vec2u16 *)&size );
			return;
		}
		case MOUSE_EVENT:
		{
			// If the mouse moved but didn't move enough to change its coordinates on the screen,
			// The event won't be sent. So there is no need to manually filter this edge case here.
			COORD const mousePos = recordedInput->Event.MouseEvent.dwMousePosition;
			mouse_update_position( *(vec2u16 *)&mousePos );
			return;
		}
		case KEY_EVENT:
		{
			if ( !recordedInput->Event.KeyEvent.bKeyDown ) return;

			enum KeyInput input;
			if ( !key_input_from_u32( recordedInput->Event.KeyEvent.wVirtualKeyCode, &input ) )
			{
				return;
			}
			console_cursor_set_position( 20, 1 );
			console_draw( L"Key input: %2u", input );

			if ( mastermind_try_consume_input( &s_mastermind, input ) )
			{
				return;
			}

			if ( input == KeyInput_ESCAPE )
			{
				s_mainLoop = false;
			}

			// Just to simplify testing
			struct Widget *widget = widget_optget( WidgetId_FRAMERATE );
/*			if ( input == KeyInput_NUMPAD_8 ) {
				widget_fpsbar_toggle_fps( widget );
			}
			if ( input == KeyInput_NUMPAD_9 ) {
				widget_fpsbar_toggle_ms( widget );
			}*/
			if ( input == KeyInput_NUMPAD_4 ) {
				widget = widget_optget( WidgetId_COUNTDOWN );
				widget_countdown_start( widget );
			}
			else if ( input == KeyInput_NUMPAD_5 ) {
				widget = widget_optget( WidgetId_COUNTDOWN );
				widget_countdown_pause( widget );
			}
			else if ( input == KeyInput_NUMPAD_6 ) {
				widget = widget_optget( WidgetId_COUNTDOWN );
				widget_countdown_resume( widget );
			}
			else if ( input == KeyInput_NUMPAD_7 ) {
				widget = widget_optget( WidgetId_COUNTDOWN );
				widget_countdown_reset( widget );
			}
			break;
		}

		// Should be ignored according to the Windows documentation
		// https://learn.microsoft.com/en-us/windows/console/input-record-str
		case FOCUS_EVENT:
		case MENU_EVENT:
		default: return;
	}
}


static bool consume_user_inputs( HANDLE const consoleHandle )
{
	assert( consoleHandle != INVALID_HANDLE_VALUE );

	DWORD nbEvents = 0;
	if ( !GetNumberOfConsoleInputEvents( consoleHandle, &nbEvents ) )
	{
		fprintf( stderr, "[ERROR]: GetNumberOfConsoleInputEvents failure. (Code %u)\n", GetLastError() );
		return false;
	}
	if ( nbEvents == 0 ) return true; // Nothing to do.

	DWORD nbInputsRead;
	INPUT_RECORD inputsBuffer[nbEvents];
	if ( !ReadConsoleInput( consoleHandle, &inputsBuffer[0], nbEvents, &nbInputsRead ) )
	{
		fprintf( stderr, "[ERROR]: ReadConsoleInput failure. (Code %u)\n", GetLastError() );
		return false;
	}

	for ( DWORD idx = 0; idx < nbInputsRead; ++idx )
	{
		consume_input( &inputsBuffer[idx] );
	}

	return true;
}


int main( void )
{
	srand( time( NULL ) );

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
	if ( !widget_global_init() )
	{
		return 3;
	}


	mastermindv2_init( &s_mastermind );
	mastermindv2_start_game( &s_mastermind );

	widget_timer_start( widget_optget( WidgetId_TIMER ) );
	
	// TODO Improve, shouldn't be a static
	while ( s_mainLoop )
	{
		consume_user_inputs( console_input_handle() );

		// Frame functions
		console_screen_frame();
		widget_frame();

		// TEMP - To move somewhere else
		console_cursor_set_position( 2, 15 );
		screenpos const mousePos = mouse_get_position();
		vec2u16 const screenSize = console_screen_get_size();
		console_draw( L" Screen: %ux%u | Mouse: %ux%u  ", screenSize.x, screenSize.y, mousePos.x, mousePos.y );

		// Refresh the game display in the console
		console_refresh();

		// Last function call in the loop
		fpscounter_frame( fpsCounter );
	}

	widget_global_uninit();
	fpscounter_uninit( fpsCounter );
	console_global_uninit();
	return 0;
}
