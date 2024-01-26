// Mastermind game in C
#include "core/core.h"
#include "mastermind.h"
#include "characters_list.h"
#include "fps_counter.h"
#include "mouse.h"
#include "gameloop.h"
#include "settings.h"
#include "random.h"
#include "keybindings.h"
#include "ui/ui.h"

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


void gameloop_emit_event( enum EventType type, struct EventData const *data )
{
/*	if ( mastermind_consume_event( type, data ) )
	{
		return;
	}
*/
/*	if ( components_consume_event( type, data ) )
	{
		return;
	}
*/
	if ( type == EventType_KEY_PRESSED /*&& data->keyPressedEvent.input == keybinding_get_binded_key( KeyBinding_QUIT )*/ )
	{
		s_mainLoop = false;
		return;
	}
}


static void gameloop_consume_key_input( enum KeyInput const input )
{
//	cursor_update_yx( 1, 16 );
//	term_write( L"Input: %2u", input );

	/*if ( components_try_consume_input( input ) )
	{
		return;
	}*/

	if ( mastermind_try_consume_input( input ) )
	{
		return;
	}

	if ( input == keybinding_get_binded_key( KeyBinding_QUIT ) )
	{
		gameloop_emit_event( EventType_STOP_EXECUTION, NULL );
		return;
	}
}


void gameloop_emit_key( enum KeyInput const input )
{
    struct EventKeyPressed const event = ( struct EventKeyPressed ) { .input = input };
    gameloop_emit_event( EventType_KEY_PRESSED, (struct EventData const *)&event );
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
				// All Numpad and Numbers are the same keys for the game, so convert numpad to its number version.
				if ( key_input_is_numpad( input ) )
				{
					input = key_input_from_numpad_to_number( input );
				}
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
	success = success && ui_init();
//	success = success && components_init();

	return success;
}


void uninit_systems( void )
{
	ui_uninit();
//	components_uninit();
	fpscounter_uninit( fpscounter_get_instance() );
	term_uninit();
}


int main( void )
{
	if ( !init_systems() )
	{
		return ExitCode_FAILURE;
	}

	ui_change_scene( UIScene_MAIN_MENU );

	while ( s_mainLoop )
	{
		consume_user_inputs();
		ui_frame();
		term_refresh();
		// Last function call in the loop
		fpscounter_frame( fpscounter_get_instance() );
	}

	uninit_systems();
	return ExitCode_SUCCESS;
}
