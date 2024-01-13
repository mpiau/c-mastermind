#include "components/component_screen_size.h"

#include "components/component_header.h"
#include "game.h"
#include "terminal/terminal.h"

#include <stdlib.h>


struct ComponentScreenSize
{
    struct ComponentHeader header;
	screensize size;
};
#define CAST_TO_COMPONENT( _header ) ( ( struct ComponentScreenSize * )( _header ) )


static void on_refresh_callback( struct ComponentHeader const *header )
{
	struct ComponentScreenSize const *comp = CAST_TO_COMPONENT( header );
	screenpos const ul = (screenpos) { .x = 10, .y = 1 };

	bool const widthSufficient = comp->size.w >= GAME_SIZE_WIDTH;
	bool const heightSufficient = comp->size.h >= GAME_SIZE_HEIGHT;

	cursor_update_pos( ul );

	if ( widthSufficient && heightSufficient )
	{
		style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT ) );
		term_write( L"Screen: %ux%u                ", comp->size.w, comp->size.h );
		return;
	}

	struct Style const errorNbColor = STYLE( FGColor_RED );
	struct Style const textColor = STYLE( FGColor_BRIGHT_BLACK );

	style_update( textColor );

	term_write( L"Screen: " );

	if ( !widthSufficient )
	{
		style_update( errorNbColor );
		term_write( L"%u", comp->size.w );
		style_update( textColor );
	}
	else
	{
		term_write( L"%u", comp->size.w );
	}

	term_write( L"x" );

	if ( !heightSufficient )
	{
		style_update( errorNbColor );
		term_write( L"%u", comp->size.h );
	}
	else
	{
		term_write( L"%u", comp->size.h );
		style_update( errorNbColor );
	}
	term_write( L" (req: %ux%u)  ", GAME_SIZE_WIDTH, GAME_SIZE_HEIGHT );
}


static void on_resize_callback( struct ComponentHeader *const header, screensize const newSize )
{
	CAST_TO_COMPONENT( header )->size = newSize;
	header->refreshNeeded = true;
}


struct ComponentHeader *component_screen_size_create( void )
{
    struct ComponentScreenSize *const comp = calloc( 1, sizeof( struct ComponentScreenSize ) );
    if ( !comp ) return NULL;

	component_make_header( &comp->header, ComponentId_SCREEN_SIZE, true );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
	callbacks->refreshCb = on_refresh_callback;
	callbacks->resizeCb = on_resize_callback;

	// Specific component data
	comp->size = term_size();

    return (struct ComponentHeader *)comp;
}
