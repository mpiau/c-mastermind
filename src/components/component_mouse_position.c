#include "components/component_mouse_position.h"

#include "widgets/widget_definition.h"
#include "terminal/terminal_screen.h"
#include "terminal/terminal_style.h"

struct ComponentMousePosition
{
    struct ComponentHeader header;

    screenpos pos;
    screenpos ul;
    struct TermStyle style;
};
#define CAST_TO_COMPONENT( _header ) ( ( struct ComponentMousePosition * )( _header ) )

static void on_mouse_move_callback( struct ComponentHeader *const widget, screenpos const pos )
{
    CAST_TO_COMPONENT( widget )->pos = pos;
    widget->refreshNeeded = true;
}


static void on_redraw_callback( struct ComponentHeader *const widget )
{
    struct ComponentMousePosition *comp = CAST_TO_COMPONENT( widget );

	term_screen_set_cursor_pos( comp->ul );
	term_style_set_current( comp->style );
	term_screen_write( L"Mouse: %ux%u  ", comp->pos.x, comp->pos.y );
}


struct ComponentHeader *component_mouse_position_create( void )
{
    struct ComponentMousePosition *const comp = calloc( 1, sizeof( struct ComponentMousePosition ) );
    if ( !comp ) return NULL;

    component_make_header( &comp->header, ComponentId_MOUSE_POSITION, true );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->mouseMoveCb = on_mouse_move_callback;
    callbacks->redrawCb = on_redraw_callback;

    // Specific to the component 
    comp->ul = (screenpos) { .x = 35, .y = 1 };
    comp->pos = mouse_get_position();
    comp->style = term_style_make( COLOR_BRIGHT_BLACK, Properties_NONE );

    return (struct ComponentHeader *)comp;
}
