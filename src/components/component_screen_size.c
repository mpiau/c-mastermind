#include "components/component_screen_size.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "console.h"
#include "console/console_screen.h"
#include "game.h"

struct ComponentScreenSize
{
    struct Widget header;

	screenpos   pos;
	struct Attr errorAttr;
	struct Attr defaultAttr;
};
#define CAST_TO_COMPONENT( _header ) ( struct ComponentScreenSize * )( _header )


static void redraw_callback( struct Widget *header )
{
	struct ComponentScreenSize const *comp = CAST_TO_COMPONENT( header );

	vec2u16 const screenSize = console_screen_get_size();
	bool const widthTooSmall = console_screen_is_width_too_small();
	bool const heightTooSmall = console_screen_is_height_too_small();
	bool const screenTooSmall = widthTooSmall || heightTooSmall;

	console_set_cpos( comp->pos );
	u32 charsDrawn = 0;

	console_set_attr( widthTooSmall ? comp->errorAttr : comp->defaultAttr );
	charsDrawn = console_write( L"%u", screenSize.w );

	console_set_attr( comp->defaultAttr );
	charsDrawn += console_write( L"x" );

	console_set_attr( heightTooSmall ? comp->errorAttr : comp->defaultAttr );
	charsDrawn += console_write( L"%u", screenSize.h );

	if ( screenTooSmall )
	{
		console_set_attr( comp->errorAttr );
		charsDrawn += console_write( L" (req: %ux%u)", GAME_SIZE_WIDTH, GAME_SIZE_HEIGHT );
	}

	// Fill the rest of the widget with spaces to remove the unwanted chars in screen
	console_write( L"%*lc", 23 - charsDrawn, L' ' );
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
	comp->errorAttr = ATTR( AttrColor_RED_FG, AttrStyle_DEFAULT, AttrShade_DEFAULT );
	comp->defaultAttr = ATTR( AttrColor_BLACK_FG, AttrStyle_DEFAULT, AttrShade_BRIGHT );

    return (struct Widget *)comp;
}
