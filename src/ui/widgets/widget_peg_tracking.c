#include "ui/widgets.h"
#include "events.h"
#include "terminal/terminal.h"
#include "mouse.h"
#include "game.h"

#include <stdlib.h>


struct WidgetPegTracking
{
    struct Widget base;

    struct Character charactersSave[2][4];
    struct Character pegSave[2][4];
    screenpos pos;
    gamepiece piece;
    bool active;
};


static void write_saved_content_at_pos( struct WidgetPegTracking *widget )
{
    for ( usize y = 0; y < 2 && widget->pos.y + y <= GAME_SIZE_HEIGHT; ++y )
    {
        for ( usize x = 0; x < 4 && widget->pos.x + x <= GAME_SIZE_WIDTH; ++x )
        {
            struct Character const pegPart = widget->pegSave[y][x];
            screenpos const pos = SCREENPOS( widget->pos.x + x, widget->pos.y + y );

            // If the character on screen is different than the peg part than we expect, we shouldn't
            // override the character with our save, because our save is obsolete.
            if ( !character_equals( term_character_buffered_at_pos( pos ), pegPart ) ) continue;

            struct Character const character = widget->charactersSave[y][x];
            cursor_update_pos( pos );
            style_update( character.style );
            term_write( L"%lc", character.unicode );
        }
    }    
}


static void save_content_at_pos( struct WidgetPegTracking *widget, bool saveCharacter )
{
    for ( usize y = 0; y < 2 && widget->pos.y + y <= GAME_SIZE_HEIGHT; ++y )
    {
        for ( usize x = 0; x < 4 && widget->pos.x + x <= GAME_SIZE_WIDTH; ++x )
        {
            screenpos pos = SCREENPOS( widget->pos.x + x, widget->pos.y + y );
            if ( saveCharacter )
            {
                widget->charactersSave[y][x] = term_character_buffered_at_pos( pos );
            }
            else
            {
                widget->pegSave[y][x] = term_character_buffered_at_pos( pos );
            }
        }
    }
}



static void update_tracking( struct WidgetPegTracking *widget, screenpos pos )
{
    write_saved_content_at_pos( widget );
    widget->pos = pos;

    save_content_at_pos( widget, true );
    piece_write_4x2( widget->pos, widget->piece );
}


static void enable_tracking( struct WidgetPegTracking *widget, gamepiece piece )
{
    widget->pos = mouse_pos();
    widget->piece = piece;
    widget->active = true;

    save_content_at_pos( widget, true );
    piece_write_4x2( widget->pos, widget->piece );
    save_content_at_pos( widget, false );
}


static void disable_tracking( struct WidgetPegTracking *widget )
{
    widget->active = false;
    write_saved_content_at_pos( widget );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetPegTracking *const widget = (struct WidgetPegTracking *)subscriber;

    switch( event->type )
    {
        case EventType_MOUSE_MOVED:
        {
            if ( widget->active )
            {
                update_tracking( widget, event->mouseMoved.pos );
            }
            break;
        }

        case EventType_PEG_SELECTED:
        {
            if ( widget->active )
            {
                disable_tracking( widget );
            }
            enable_tracking( widget, event->peg.piece );
            break;
        }
        case EventType_PEG_UNSELECTED:
        {
            disable_tracking( widget );
            break;
        }
    }

    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
    struct WidgetPegTracking *const widget = (struct WidgetPegTracking *)base;
    widget->active = false;
}


static void disable_callback( struct Widget *base )
{
    struct WidgetPegTracking *const widget = (struct WidgetPegTracking *)base;
    if ( widget->active )
    {
        disable_tracking( widget );
    }
}


struct Widget *widget_peg_tracking_create( void )
{
    struct WidgetPegTracking *const widget = calloc( 1, sizeof( struct WidgetPegTracking ) );
    if ( !widget ) return NULL;

    widget->base.name = "PegTracking";
    widget->base.enabledScenes = UIScene_IN_GAME;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    event_register( (struct Widget *)widget, on_event_callback );
    event_subscribe( (struct Widget *)widget, EventType_MaskAll );

    return (struct Widget *)widget;
}
