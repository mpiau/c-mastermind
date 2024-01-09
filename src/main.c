// Mastermind game in C
#include "mastermind.h"
#include "console.h"
#include "console/console_screen.h"
#include "characters_list.h"
#include "core_unions.h"
#include "fps_counter.h"
#include "widgets/widget_timer.h"
#include "widgets/widget_countdown.h"
#include "widgets/widget_utils.h"
#include "mouse.h"
#include "gameloop.h"
#include "settings.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <windows.h>

static bool s_mainLoop = true;

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

/*char const *get_game_name( void )
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
}*/


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


static void gameloop_consume_key_input( enum KeyInput const input )
{
	console_cursor_set_position( 1, 16 );
	console_draw( L"Input: %2u", input );

	if ( input == KeyInput_ESCAPE || input == KeyInput_Q )
	{
		s_mainLoop = false;
		return;
	}

	if ( widget_try_consume_input( input ) )
	{
		return;
	}

	if ( mastermind_try_consume_input( input ) )
	{
		return;
	}
}


void gameloop_emit_key( enum KeyInput const input )
{
	gameloop_consume_key_input( input );
}


static void consume_input( INPUT_RECORD const *const recordedInput )
{
	assert( recordedInput );

	switch ( recordedInput->EventType )
	{
		case WINDOW_BUFFER_SIZE_EVENT:
		{
			COORD const size = recordedInput->Event.WindowBufferSizeEvent.dwSize;
			console_screen_resize( *(vec2u16 *)&size );
			console_on_screen_resize( *(vec2u16 *)&size );
			return;
		}
		case MOUSE_EVENT:
		{
			mouse_consume_event( &recordedInput->Event.MouseEvent );
			return;
		}
		case KEY_EVENT:
		{
			if ( !recordedInput->Event.KeyEvent.bKeyDown ) return;

			enum KeyInput input;
			if ( key_input_from_u32( recordedInput->Event.KeyEvent.wVirtualKeyCode, &input ) )
			{
				gameloop_consume_key_input( input );
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

	// TODO: Perhaps have a limit of the number of inputs to acknowledge by frame
	// And discard the rest to not take too much delay between the pressed input and the game update.
	// Example: If the user spam keys, we will discard some of them to stay on track

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
		// TODO have a filelog for these. The console is already used for the game.
		fprintf( stderr, "[FATAL ERROR]: Failed to init the console. Aborting." );
		return 1; // TODO improve these errors code.
	}

	struct FPSCounter *const fpsCounter = fpscounter_init();
	if ( fpsCounter == NULL )
	{
		fprintf( stderr, "[FATAL ERROR]: Failed to init FPS Counter. Aborting." );
		return 2;
	}

	settings_global_init();
	mouse_init();
	if ( !widget_global_init() )
	{
		return 3;
	}

	while ( s_mainLoop )
	{
		consume_user_inputs( console_input_handle() );

		// Frame functions
		console_screen_frame();
		widget_frame();

		// TEMP - To move somewhere else
		console_set_pos( SCREENPOS( 35, 1 ) );
		console_set_style( style_make( ColorFG_BLACK, ColorBrightness_FG, DispAttr_NONE ) );
		screenpos const mousePos = mouse_get_position();
		console_write( L" | Mouse: %ux%u  ", mousePos.x, mousePos.y );
//		console_cursor_set_position( 1, 35 );
//		console_color_reset();
//		screenpos const mousePos = mouse_get_position();
//		console_draw( L" | Mouse: %ux%u  ", mousePos.x, mousePos.y );

		// Refresh the game display in the console
		console_refresh_v2();

		// Last function call in the loop
		fpscounter_frame( fpsCounter );
	}

	widget_global_uninit();
	fpscounter_uninit( fpsCounter );
	console_global_uninit();
	return 0;
}
