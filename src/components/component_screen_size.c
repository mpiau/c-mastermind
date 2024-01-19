#include "components/component_screen_size.h"

#include "components/component_header.h"
#include "game.h"
#include "terminal/terminal.h"

#include <stdlib.h>

struct ComponentScreenSize
{
    struct ComponentHeader header;
	screensize size;
	screenpos ulPos;
	screenpos sizePos;
	struct Style errorStyle;
	struct Style defaultStyle;
	bool lastResizeSufficient;
};
#define CAST_TO_COMP( _header ) ( ( struct ComponentScreenSize * )( _header ) )


static void write_size_update( struct ComponentScreenSize *comp )
{
	bool const widthSufficient = comp->size.w >= GAME_SIZE_WIDTH;
	bool const heightSufficient = comp->size.h >= GAME_SIZE_HEIGHT;

	cursor_update_pos( comp->sizePos );
	style_update( comp->defaultStyle );

	if ( widthSufficient && heightSufficient )
	{
		term_write( L"%ux%u", comp->size.w, comp->size.h );
		if ( !comp->lastResizeSufficient )
		{
			// For the required message removal.
			term_write( L"                " );
			comp->lastResizeSufficient = true;
		}
		return;
	}

	comp->lastResizeSufficient = false;
	if ( !widthSufficient )
	{
		style_update( comp->errorStyle );
		term_write( L"%u", comp->size.w );
		style_update( comp->defaultStyle );
	}
	else
	{
		term_write( L"%u", comp->size.w );
	}

	term_write( L"x" );

	if ( !heightSufficient )
	{
		style_update( comp->errorStyle );
		term_write( L"%u", comp->size.h );
	}
	else
	{
		term_write( L"%u", comp->size.h );
		style_update( comp->errorStyle );
	}
	term_write( L" (req: %ux%u)  ", GAME_SIZE_WIDTH, GAME_SIZE_HEIGHT );
}


static void on_resize_callback( struct ComponentHeader *header, screensize const newSize )
{
	struct ComponentScreenSize *comp = CAST_TO_COMP( header );
	if ( comp->size.raw == newSize.raw )
		return;

	comp->size = newSize;
	write_size_update( comp );
}


static void enable_callback( struct ComponentHeader *header )
{
    struct ComponentScreenSize *comp = CAST_TO_COMP( header );

	comp->size = term_size();
	comp->lastResizeSufficient = false;

	cursor_update_pos( comp->ulPos );
	style_update( comp->defaultStyle );
	int const written = term_write( L"Screen: " );
	comp->sizePos = SCREENPOS( comp->ulPos.x + written, comp->ulPos.y );

	write_size_update( comp );
}


static void disable_callback( struct ComponentHeader *header )
{
    struct ComponentScreenSize *comp = CAST_TO_COMP( header );
	cursor_update_pos( comp->ulPos );
	term_write( L"                                " );
}


struct ComponentHeader *component_screen_size_create( void )
{
    struct ComponentScreenSize *const comp = calloc( 1, sizeof( struct ComponentScreenSize ) );
    if ( !comp ) return NULL;

//	component_make_header( &comp->header, ComponentId_SCREEN_SIZE, true );

	comp->header.id = ComponentId_SCREEN_SIZE;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->enableCb = enable_callback;
    callbacks->disableCb = disable_callback;
	callbacks->resizeCb = on_resize_callback;

	comp->errorStyle = STYLE( FGColor_RED );
	comp->defaultStyle = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );
	comp->ulPos = (screenpos) { .x = 10, .y = 1 };

    return (struct ComponentHeader *)comp;
}
