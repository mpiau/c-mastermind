#include "widgets/widget.h"
#include "widgets/widget_definition.h"

#include "widgets/widget_utils.h"
#include "widgets/widget_board.h"
#include "components/component_game_buttons.h"
#include "components/component_framerate.h"
#include "components/component_screen_size.h"
#include "components/component_summary.h"
#include "widgets/widget_timer.h"
#include "widgets/widget_countdown.h"
#include "widgets/widget_peg_selection.h"

#include "console/console_screen.h"

#include <stdlib.h>

static struct Widget *s_widgets[ComponentId_Count] = {};

// Array with the list of ID for the priority frame / priority input


static void on_mouse_mouvement_callback( screenpos const oldPos, screenpos const newPos )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget && widget->callbacks.mouseMoveCb )
        {
            widget->callbacks.mouseMoveCb( widget, oldPos, newPos );
        }
    }
}

static void on_mouse_click_callback( screenpos const mousePos, enum MouseButton button )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget && widget->callbacks.mouseClickCb )
        {
            widget->callbacks.mouseClickCb( widget, mousePos, button );
        }
    }
}

static void on_screen_resize_callback( vec2u16 const oldSize, vec2u16 const newSize )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget == NULL ) continue;

		if ( widget->callbacks.resizeCb != NULL )
		{
			widget->callbacks.resizeCb( widget, oldSize, newSize );
		}
    }
}

static void on_game_update_callback( struct Mastermind const *mastermind, enum GameUpdateType const updateType )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];

        if ( widget && widget->callbacks.gameUpdateCb )
        {
            widget->callbacks.gameUpdateCb( widget, mastermind, updateType );
        }
    }
}


bool widget_global_init( void )
{
    s_widgets[ComponentId_FRAMERATE] = component_framerate_create();
	s_widgets[ComponentId_SCREEN_SIZE] = component_screen_size_create();
	s_widgets[ComponentId_GAME_BUTTONS] = component_game_buttons_create();
	s_widgets[ComponentId_BOARD] = widget_board_create();
    s_widgets[ComponentId_TIMER] = widget_timer_create();
	s_widgets[ComponentId_SUMMARY] = component_summary_create();
	s_widgets[ComponentId_PEG_SELECTION] = widget_peg_selection_create();
	// boardSummary
    // s_widgets[ComponentId_COUNTDOWN] = widget_countdown_create();
    // Init others widgets [...]

    // Register the widgets on event based updates (mouse, keyboard, resize, ...)
    mouse_register_on_mouse_mouvement_callback( on_mouse_mouvement_callback );
    mouse_register_on_mouse_click_callback( on_mouse_click_callback );
    console_screen_register_on_resize_callback( on_screen_resize_callback );
	mastermind_register_update_callback( on_game_update_callback );
    // TODO add keyboard input
    return true;
}


void widget_global_uninit( void )
{
    for ( enum ComponentId idx = 0; idx < ComponentId_Count; ++idx )
    {
        if ( s_widgets[idx] )
        {
            free( s_widgets[idx] );
        }
    }
}


bool widget_exists( enum ComponentId const id )
{
    return s_widgets[id] != NULL;
}


struct Widget *widget_optget( enum ComponentId id )
{
    return s_widgets[id];
}


// /////////////////////////////////////////////////////////

void widget_frame( void )
{
    for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct Widget *const widget = s_widgets[id];
		if ( widget == NULL ) continue;

        if ( widget->callbacks.frameCb != NULL )
        {
            widget->callbacks.frameCb( widget );
		}

		if ( widget->redrawNeeded )
		{
			if ( widget->callbacks.redrawCb != NULL )
			{
				widget->callbacks.redrawCb( widget );
			}
			widget->redrawNeeded = false;
		}
   }
}


bool widget_try_consume_input( enum KeyInput const input )
{
	for ( enum ComponentId id = 0; id < ComponentId_Count; ++id )
    {
        struct Widget *const widget = s_widgets[id];
		if ( widget == NULL ) continue;
        if ( widget->callbacks.inputReceivedCb == NULL ) continue;

		if ( widget->callbacks.inputReceivedCb( widget, input ) )
		{
			return true;
		}
	}
	return false;
}


void widget_set_header( struct Widget *const header, enum ComponentId const id, bool const enabled )
{
	header->id = id;
	header->enabled = enabled;
	header->redrawNeeded = enabled;
}