#include "components/component_screen_size.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "console.h"
#include "console_screen.h"
#include "game.h"

struct ComponentScreenSize
{
    struct Widget header;

	screenpos pos;
};
#define CAST_TO_COMPONENT( _header ) ( struct ComponentScreenSize * )( _header )


static void redraw_callback( struct Widget *header )
{
	struct ComponentScreenSize *comp = CAST_TO_COMPONENT( header );

	vec2u16 const screenSize = console_screen_get_size();
	enum ConsoleColorFG const defaultColor = ConsoleColorFG_BRIGHT_BLACK;
	enum ConsoleColorFG const errorColor   = ConsoleColorFG_RED;
	enum ConsoleColorFG const widthColor   = console_screen_is_width_too_small() ? errorColor : defaultColor;
	enum ConsoleColorFG const heightColor  = console_screen_is_height_too_small() ? errorColor : defaultColor;

    console_cursor_setpos( comp->pos );
	u32 charsDrawn = 0;

	console_color_fg( widthColor );
	charsDrawn = console_draw( L"%u", screenSize.w );

	console_color_fg( defaultColor );
	charsDrawn += console_draw( L"x" );

	console_color_fg( heightColor );
	charsDrawn += console_draw( L"%u", screenSize.h );

	if ( heightColor == errorColor || widthColor == errorColor )
	{
		console_color_fg( errorColor );
		charsDrawn += console_draw( L" (req: %ux%u)", GAME_SIZE_WIDTH, GAME_SIZE_HEIGHT );
	}

	// Fill the rest of the widget with spaces to remove the unwanted chars in screen
	console_draw( L"%*lc", 23 - charsDrawn, L' ' );
	
	console_color_reset();
}


static void on_resize_callback( struct Widget *widget, vec2u16 const oldSize, vec2u16 const newSize )
{
	widget->redrawNeeded = true;
}


struct Widget *component_screen_size_create( void )
{
    struct ComponentScreenSize *const comp = calloc( 1, sizeof( struct ComponentScreenSize ) );
    if ( !comp ) return NULL;

	widget_set_header( &comp->header, ComponentId_SCREEN_SIZE, true );

    struct WidgetCallbacks *const callbacks = &comp->header.callbacks;
	callbacks->redrawCb = redraw_callback;
	callbacks->resizeCb = on_resize_callback;

	// Specific component data
	comp->pos = SCREENPOS( 10, 1 );

    return (struct Widget *)comp;
}
