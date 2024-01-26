#include "ui/widgets.h"
#include "game.h"
#include "terminal/terminal.h"
#include "events.h"

#include <stdlib.h>

struct WidgetScreenSize
{
    struct Widget base;

    screenpos prefixPos;
    screenpos pos;

    struct Style defStyle;
    struct Style errStyle;
};


static void write_size_update( struct WidgetScreenSize *widget, screensize const size )
{
	bool const widthSufficient = size.w >= GAME_SIZE_WIDTH;
	bool const heightSufficient = size.h >= GAME_SIZE_HEIGHT;

	cursor_update_pos( widget->pos );
	style_update( widget->defStyle );

	if ( widthSufficient && heightSufficient )
	{
		term_write( L"%ux%u                ", size.w, size.h );
		return;
	}

	if ( !widthSufficient )
	{
		style_update( widget->errStyle );
		term_write( L"%u", size.w );
		style_update( widget->defStyle );
	}
	else
	{
		term_write( L"%u", size.w );
	}

	term_write( L"x" );

	if ( !heightSufficient )
	{
		style_update( widget->errStyle );
		term_write( L"%u", size.h );
	}
	else
	{
		term_write( L"%u", size.h );
		style_update( widget->errStyle );
	}
	term_write( L" (req: %ux%u)  ", GAME_SIZE_WIDTH, GAME_SIZE_HEIGHT );
}


static enum EventPropagation on_event_callback( void *subscriber, struct Event const *event )
{
    if ( event->type == EventType_SCREEN_RESIZED )
    {
        write_size_update( (struct WidgetScreenSize *)subscriber, event->screenResized.size );
    }

    return EventPropagation_CONTINUE;
}


static void enable_callback( struct Widget *base )
{
    struct WidgetScreenSize *widget = (struct WidgetScreenSize *)base;

	screensize const currSize = term_size();

	cursor_update_pos( widget->prefixPos );
	style_update( widget->defStyle );
	int const written = term_write( L"Screen: " );
	widget->pos = SCREENPOS( widget->prefixPos.x + written, widget->prefixPos.y );

	write_size_update( widget, currSize );
}


static void disable_callback( struct Widget *base )
{
    struct WidgetScreenSize *widget = (struct WidgetScreenSize *)base;

  	cursor_update_pos( widget->prefixPos );
    style_update( widget->defStyle );
	term_write( L"                                " );
}


struct Widget *widget_screensize_create( void )
{
    struct WidgetScreenSize *const widget = calloc( 1, sizeof( struct WidgetScreenSize ) );
    if ( !widget ) return NULL;

    widget->base.name = "ScreenSize";
    widget->base.enabledScenes = UIScene_ALL;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    event_register( (struct Widget *)widget, on_event_callback );
    event_subscribe( (struct Widget *)widget, EventType_SCREEN_RESIZED );

    widget->defStyle = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
    widget->errStyle = STYLE( FGColor_RED );
	widget->prefixPos = (screenpos) { .x = 10, .y = 1 };

    return (struct Widget *)widget;
}
