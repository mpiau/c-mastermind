#include "components/component_mouse_position.h"

#include "components/component_header.h"
#include "terminal/terminal.h"
#include "rect.h"

#include <stdlib.h>


struct ComponentMousePosition
{
    struct ComponentHeader header;

    struct Rect rect;
    struct Style style;
};
#define CAST_TO_COMP( _header ) ( ( struct ComponentMousePosition * )( _header ) )


static void write_mouse_pos( struct ComponentMousePosition *comp, screenpos const pos )
{
	cursor_update_pos( rect_get_ul_corner( &comp->rect ) );
	style_update( comp->style );
    term_write( L"M:%ux%u  ", pos.x, pos.y );
}

static void on_mouse_move_callback( struct ComponentHeader *header, screenpos const pos )
{
    struct ComponentMousePosition *comp = CAST_TO_COMP( header );
	write_mouse_pos( comp, pos );
}

static void enable_callback( struct ComponentHeader *header )
{
    struct ComponentMousePosition *comp = CAST_TO_COMP( header );
    write_mouse_pos( comp, SCREENPOS( 0, 0 ) );
}


static void disable_callback( struct ComponentHeader *header )
{
    struct ComponentMousePosition *comp = CAST_TO_COMP( header );
    rect_clear( &comp->rect );
}

struct ComponentHeader *component_mouse_position_create( void )
{
    struct ComponentMousePosition *const comp = calloc( 1, sizeof( struct ComponentMousePosition ) );
    if ( !comp ) return NULL;

    comp->header.id = ComponentId_MOUSE_POSITION;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->enableCb = enable_callback;
    callbacks->disableCb = disable_callback;
    callbacks->mouseMoveCb = on_mouse_move_callback;

    // Specific to the component 
    comp->rect = rect_make( SCREENPOS( 41, 1 ), VEC2U16( 11, 1 ) );
    comp->style = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );

    return (struct ComponentHeader *)comp;
}
