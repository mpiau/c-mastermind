// Mastermind game in C
#include "core/core.h"
#include "mastermind.h"
#include "characters_list.h"
#include "fps_counter.h"
#include "mouse.h"
#include "gameloop.h"
#include "settings.h"
#include "random.h"

#include "components/components.h"
#include "terminal/terminal.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

enum ExitCode
{
	ExitCode_SUCCESS,
	ExitCode_FAILURE
};

static bool s_mainLoop = true;

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
//	cursor_update_yx( 1, 16 );
//	term_write( L"Input: %2u", input );

	if ( input == KeyInput_ESCAPE || input == KeyInput_Q )
	{
		s_mainLoop = false;
		return;
	}

	if ( components_try_consume_input( input ) )
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
			term_on_resize( *(screensize *)&size );
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


static bool consume_user_inputs( void )
{
	DWORD nbEvents = 0;
	if ( !GetNumberOfConsoleInputEvents( term_input_handle(), &nbEvents ) )
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
	if ( !ReadConsoleInput( term_input_handle(), &inputsBuffer[0], nbEvents, &nbInputsRead ) )
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


bool init_systems( void )
{
	bool success = true;

	success = success && random_init();
	success = success && term_init( "Mastermind", true );
	success = success && ( fpscounter_init() != NULL );
	success = success && settings_init();
	success = success && mouse_init();
	success = success && components_init();

	return success;
}


void uninit_systems( void )
{
	components_uninit();
	fpscounter_uninit( fpscounter_get_instance() );
	term_uninit();
}


int main( void )
{
	if ( !init_systems() )
	{
		return ExitCode_FAILURE;
	}

	while ( s_mainLoop )
	{
		consume_user_inputs();
		components_frame();
		term_refresh();
		// Last function call in the loop
		fpscounter_frame( fpscounter_get_instance() );
	}

	uninit_systems();
	return ExitCode_SUCCESS;
}
