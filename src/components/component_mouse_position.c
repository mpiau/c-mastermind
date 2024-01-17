#include "components/component_mouse_position.h"

#include "components/component_header.h"
#include "mouse.h"
#include "terminal/terminal.h"

#include <stdlib.h>


struct ComponentMousePosition
{
    struct ComponentHeader header;

    screenpos pos;
    screenpos ul;
    struct Style style;
};
#define CAST_TO_COMP( _header ) ( ( struct ComponentMousePosition * )( _header ) )


static void on_mouse_move_callback( struct ComponentHeader *header, screenpos const pos )
{
    struct ComponentMousePosition *comp = CAST_TO_COMP( header );
    comp->pos = pos;

	cursor_update_pos( comp->ul );
	style_update( comp->style );
	term_write( L"Mouse: %ux%u  ", comp->pos.x, comp->pos.y );
}


struct ComponentHeader *component_mouse_position_create( void )
{
    struct ComponentMousePosition *const comp = calloc( 1, sizeof( struct ComponentMousePosition ) );
    if ( !comp ) return NULL;

    component_make_header( &comp->header, ComponentId_MOUSE_POSITION, true );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->mouseMoveCb = on_mouse_move_callback;

    // Specific to the component 
    comp->ul = (screenpos) { .x = 41, .y = 1 };
    comp->pos = mouse_pos();
    comp->style = STYLE( FGColor_BRIGHT_BLACK );

    return (struct ComponentHeader *)comp;
}
