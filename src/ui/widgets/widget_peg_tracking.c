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
    struct Peg peg;
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
    peg_write_4x2( widget->pos, widget->peg );
}


static void disable_tracking( struct WidgetPegTracking *widget )
{
    if ( widget->peg.id == PegId_EMPTY ) return;

    widget->peg.id = PegId_EMPTY;
    write_saved_content_at_pos( widget );
}


static void enable_tracking( struct WidgetPegTracking *widget, struct Peg const peg )
{
    if ( widget->peg.id != PegId_EMPTY )
    {
        disable_tracking( widget );
    }
    widget->pos = mouse_pos();
    widget->peg = peg;

    save_content_at_pos( widget, true );
    peg_write_4x2( widget->pos, widget->peg );
    save_content_at_pos( widget, false );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    struct WidgetPegTracking *const widget = (struct WidgetPegTracking *)subscriber;

    switch( event->type )
    {
        case EventType_MOUSE_MOVED:
        {
            if ( widget->peg.id != PegId_EMPTY )
            {
                update_tracking( widget, event->mouseMoved.pos );
            }
            break;
        }

        case EventType_PEG_SELECTED:
        {
            enable_tracking( widget, event->peg.peg );
            break;
        }
        case EventType_PEG_UNSELECTED:
        {
            disable_tracking( widget );
            break;
        }
        default: break;
    }

    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
    struct WidgetPegTracking *const widget = (struct WidgetPegTracking *)base;
    widget->peg.id = PegId_EMPTY;

    event_subscribe( (struct Widget *)widget, EventType_MaskAll );
}


static void disable_callback( struct Widget *base )
{
    struct WidgetPegTracking *const widget = (struct WidgetPegTracking *)base;
    if ( widget->peg.id != PegId_EMPTY )
    {
        disable_tracking( widget );
    }

    event_unsubscribe( (struct Widget *)widget, EventType_MaskAll );
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

    return (struct Widget *)widget;
}
