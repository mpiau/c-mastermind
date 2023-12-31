#include "widgets/widget.h"
#include "widgets/widget_definition.h"

#include "widgets/widget_border.h"
#include "widgets/widget_utils.h"
#include "widgets/widget_game.h"
#include "widgets/widget_board.h"
#include "widgets/widget_board_buttons.h"
#include "widgets/widget_board_summary.h"
#include "widgets/widget_timer.h"
#include "widgets/widget_framerate.h"
#include "widgets/widget_countdown.h"

#include "console_screen.h"

#include <stdlib.h>

static struct Widget *s_widgets[WidgetId_Count] = {};


static void clear_rect( screenpos const upleft, vec2u16 const size, bool const borderOnly )
{
	for ( u16 y = 0; y < size.y; ++y )
	{
		console_cursor_set_position( upleft.y + y, upleft.x );

		if ( !borderOnly || ( y == 0 || y == size.y - 1 ) )
		{
			console_draw( L"%*lc", size.x, L' ' );
		}
		else
		{
			console_draw( L"%lc", L' ' );
			console_cursor_move_right_by( size.x - 2 );
			console_draw( L"%lc", L' ' );
		}

	}
}


static void clear_content( struct WidgetBox const *box )
{
	clear_rect( box->contentUpLeft, widget_content_get_size( box ), false );
}

static void clear_borders( struct WidgetBox const *box )
{
	clear_rect( box->borderUpLeft, widget_border_get_size( box ), true );
}

static void clear_widget( struct WidgetBox const *box )
{
	clear_rect( box->borderUpLeft, widget_border_get_size( box ), false );
}


// Array with the list of ID for the priority frame / priority input


static void on_mouse_mouvement_callback( screenpos const oldPos, screenpos const newPos )
{
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( widget && widget->callbacks.mouseMoveCb )
        {
            widget->callbacks.mouseMoveCb( widget, oldPos, newPos );
        }
    }
}


static void on_screen_resize_callback( vec2u16 const oldSize, vec2u16 const newSize )
{
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
        if ( !widget ) continue;

		enum WidgetTruncatedStatus oldStatus = widget->box.truncatedStatus;
		widget_utils_calculate_truncation( &widget->box, newSize );
		enum WidgetTruncatedStatus newStatus = widget->box.truncatedStatus;

		// TODO, this function should only mark as redrawNeeded
		// A function could be called on redrawNeeded that will check for borders + content

		if ( newStatus == WidgetTruncatedStatus_NONE && widget->box.borderOption == WidgetBorderOption_ALWAYS_VISIBLE )
		{
            widget_utils_draw_borders( &widget->box );
        }
		else if ( newStatus != WidgetTruncatedStatus_NONE && widget->box.borderOption != WidgetBorderOption_INVISIBLE )
		{
            widget_utils_draw_borders( &widget->box );
        }

		if ( oldStatus == WidgetTruncatedStatus_NONE && newStatus != WidgetTruncatedStatus_NONE )
		{
			if ( widget->id != WidgetId_GAME && !widget_is_out_of_bounds( &widget->box ) ) clear_content( &widget->box );
		}
		else if ( oldStatus != WidgetTruncatedStatus_NONE && newStatus == WidgetTruncatedStatus_NONE )
		{
			widget->redrawNeeded = true;
			if ( widget->box.borderOption == WidgetBorderOption_VISIBLE_ON_TRUNCATE )
			{
				clear_borders( &widget->box );
			}
		}
    }
}


bool widget_global_init( void )
{
    s_widgets[WidgetId_GAME] = widget_game_create();
    s_widgets[WidgetId_FRAMERATE] = widget_framerate_create();
	s_widgets[WidgetId_BOARD] = widget_board_create();
    s_widgets[WidgetId_TIMER] = widget_timer_create();
	s_widgets[WidgetId_BOARD_SUMMARY] = widget_board_summary_create();
	s_widgets[WidgetID_BOARD_BUTTONS] = widget_board_buttons_create();
	// boardSummary
    // s_widgets[WidgetId_COUNTDOWN] = widget_countdown_create();
    // Init others widgets [...]

    // Register the widgets on event based updates (mouse, keyboard, resize, ...)
    mouse_register_on_mouse_mouvement_callback( on_mouse_mouvement_callback );
    console_screen_register_on_resize_callback( on_screen_resize_callback );
    // TODO add keyboard input
    return true;
}


void widget_global_uninit( void )
{
    for ( enum WidgetId idx = 0; idx < WidgetId_Count; ++idx )
    {
        if ( s_widgets[idx] )
        {
            free( s_widgets[idx] );
        }
    }
}


bool widget_exists( enum WidgetId const id )
{
    return s_widgets[id] != NULL;
}


struct Widget *widget_optget( enum WidgetId id )
{
    return s_widgets[id];
}


// /////////////////////////////////////////////////////////


void widget_frame( void )
{
    // Need to define an order of frame, because some popups can be put on top of the others
    for ( enum WidgetId id = 0; id < WidgetId_Count; ++id )
    {
        struct Widget *widget = s_widgets[id];
		if ( !widget ) continue;

		if ( widget->redrawNeeded && !console_screen_is_being_resized() && !widget_is_out_of_bounds( &widget->box ) && !widget_is_truncated( &widget->box ) )
		{
			if ( widget->callbacks.redrawCb  )
			{
				widget->callbacks.redrawCb( widget );
			}
			widget->redrawNeeded = false;
		}

        if ( widget->callbacks.frameCb )
        {
            widget->callbacks.frameCb( widget );
		}
    }
}
