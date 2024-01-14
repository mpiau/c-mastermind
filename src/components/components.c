#include "components/components.h"
#include "components/component_header.h"

#include "widgets/widget_board.h"
#include "components/component_game_buttons.h"
#include "components/component_framerate.h"
#include "components/component_screen_size.h"
#include "components/component_summary.h"
#include "components/component_mouse_position.h"
#include "components/component_timer.h"
#include "widgets/widget_countdown.h"
#include "widgets/widget_peg_selection.h"
#include "keyboard_inputs.h"
#include "mastermind.h"
#include "mouse.h"

#include <stdlib.h>

static struct ComponentHeader *s_headers[ComponentId_Count] = {};

// Array with the list of ID for the priority frame / priority input


static void on_mouse_move_callback( screenpos const pos )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct ComponentHeader *const header = s_headers[id];
        if ( header && header->callbacks.mouseMoveCb )
        {
            header->callbacks.mouseMoveCb( header, pos );
        }
    }
}


void components_on_screen_resize( screensize const size )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct ComponentHeader *const header = s_headers[id];
        if ( header && header->callbacks.resizeCb )
		{
			header->callbacks.resizeCb( header, size );
		}
    }
}


static void on_game_update_callback( enum GameUpdateType const updateType )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct ComponentHeader *header = s_headers[id];

        if ( header && header->callbacks.gameUpdateCb )
        {
            header->callbacks.gameUpdateCb( header, updateType );
        }
    }
}


bool components_init( void )
{
    s_headers[ComponentId_FRAMERATE] = component_framerate_create();
	s_headers[ComponentId_SCREEN_SIZE] = component_screen_size_create();
	s_headers[ComponentId_MOUSE_POSITION] = component_mouse_position_create();
	s_headers[ComponentId_GAME_BUTTONS] = component_game_buttons_create();
	s_headers[ComponentId_BOARD] = widget_board_create();
    s_headers[ComponentId_TIMER] = component_timer_create();
	s_headers[ComponentId_SUMMARY] = component_summary_create();
	s_headers[ComponentId_PEG_SELECTION] = component_peg_selection_create();
	// boardSummary
    // s_components[CompId_COUNTDOWN] = widget_countdown_create();
    // Init others widgets [...]

    // Register the widgets on event based updates (mouse, keyboard, resize, ...)
    mouse_register_on_mouse_move_callback( on_mouse_move_callback );
	mastermind_register_update_callback( on_game_update_callback );
    // TODO add keyboard input
    return true;
}


void components_uninit( void )
{
    for ( enum ComponentId idx = 0; idx < ComponentId_Count; ++idx )
    {
        if ( s_headers[idx] )
        {
            free( s_headers[idx] );
            s_headers[idx] = NULL;
        }
    }
}


bool component_exists( enum ComponentId const id )
{
    return s_headers[id] != NULL;
}


struct ComponentHeader *component_try_get( enum ComponentId id )
{
    return s_headers[id];
}


// /////////////////////////////////////////////////////////


void components_frame( void )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct ComponentHeader *const header = s_headers[id];
		if ( header == NULL ) continue;

        if ( header->callbacks.frameCb != NULL )
        {
            header->callbacks.frameCb( header );
		}

		if ( header->refreshNeeded )
		{
			if ( header->callbacks.refreshCb != NULL )
			{
				header->callbacks.refreshCb( header );
			}
			header->refreshNeeded = false;
		}
   }
}


bool components_try_consume_input( enum KeyInput const input )
{
    // The components displayed on top are at the end of the list.
	for ( int id = ComponentId_Count - 1; id >= 0; --id )
    {
        struct ComponentHeader *const header = s_headers[id];

		if ( !header || !header->callbacks.inputReceivedCb )
            continue;

		if ( header->callbacks.inputReceivedCb( header, input ) )
		{
            // Input has been consumed by the component, stop.
			return true;
		}
	}
	return false;
}


void component_make_header( struct ComponentHeader *const header, enum ComponentId const id, bool const enabled )
{
	header->id = id;
	header->enabled = enabled;
	header->refreshNeeded = enabled;
}