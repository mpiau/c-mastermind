#include "components/component_screen_size.h"

#include "terminal/terminal_screen.h"
#include "terminal/terminal_style.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "game.h"

struct ComponentScreenSize
{
    struct ComponentHeader header;
	screensize size;
};
#define CAST_TO_COMPONENT( _header ) ( ( struct ComponentScreenSize * )( _header ) )


static void on_refresh_callback( struct ComponentHeader *header )
{
	struct ComponentScreenSize const *comp = CAST_TO_COMPONENT( header );
	screenpos const ul = (screenpos) { .x = 10, .y = 1 };

	bool const widthSufficient = comp->size.w >= GAME_SIZE_WIDTH;
	bool const heightSufficient = comp->size.h >= GAME_SIZE_HEIGHT;

	term_screen_set_cursor_pos( ul );

	if ( widthSufficient && heightSufficient )
	{
		term_style_set_current( term_style_make( COLOR_BRIGHT_BLACK, Properties_FAINT ) );
		term_screen_write( L"Screen: %ux%u                ", comp->size.w, comp->size.h );
		return;
	}

	struct TermStyle const errorNbColor = term_style_make( ForegroundColor_RED, Properties_NONE );
	struct TermStyle const textColor = term_style_make( COLOR_BRIGHT_BLACK, Properties_NONE );

	term_screen_set_cursor_pos( ul );
	term_style_set_current( textColor );

	term_screen_write( L"Screen: " );

	if ( !widthSufficient )
	{
		term_style_set_current( errorNbColor );
		term_screen_write( L"%u", comp->size.w );
		term_style_set_current( textColor );
	}
	else
	{
		term_screen_write( L"%u", comp->size.w );
	}

	term_screen_write( L"x" );

	if ( !heightSufficient )
	{
		term_style_set_current( errorNbColor );
		term_screen_write( L"%u", comp->size.h );
	}
	else
	{
		term_screen_write( L"%u", comp->size.h );
		term_style_set_current( errorNbColor );
	}
	term_screen_write( L" (req: %ux%u)  ", GAME_SIZE_WIDTH, GAME_SIZE_HEIGHT );
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
	callbacks->redrawCb = on_refresh_callback;
	callbacks->resizeCb = on_resize_callback;

	// Specific component data
	comp->size = term_screen_current_size();

    return (struct ComponentHeader *)comp;
}
