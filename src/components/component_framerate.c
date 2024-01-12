#include "components/component_framerate.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "fps_counter.h"
#include "mouse.h"

#include "terminal/terminal_screen.h"
#include "terminal/terminal_style.h"

struct ComponentFramerate
{
    struct ComponentHeader header;

    screenpos   pos;
    usize       lastAverageFPS;
    struct TermStyle style;
};
#define CAST_TO_COMPONENT( _header ) ( ( struct ComponentFramerate * )( _header ) )

#define RETURN_IF_DISABLED( _header )                   \
    do { if ( !_header->enabled ) return; } while ( 0 )


static void redraw_callback( struct ComponentHeader *header )
{
    RETURN_IF_DISABLED( header );

    struct ComponentFramerate const *comp = CAST_TO_COMPONENT( header );

    term_screen_set_cursor_pos( comp->pos );
    term_style_set_current( comp->style );
    term_screen_write( L"%3uFPS", comp->lastAverageFPS );
}


static void frame_callback( struct ComponentHeader *header )
{
    RETURN_IF_DISABLED( header );

    struct ComponentFramerate *comp = CAST_TO_COMPONENT( header );

    usize const lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    if ( lastAverageFPS != comp->lastAverageFPS )
    {
        comp->lastAverageFPS = lastAverageFPS;
        header->refreshNeeded = true;
    }
}


struct ComponentHeader *component_framerate_create( void )
{
    struct ComponentFramerate *const comp = calloc( 1, sizeof( struct ComponentFramerate ) );
    if ( !comp ) return NULL;

    component_make_header( &comp->header, ComponentId_FRAMERATE, true );

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->frameCb = frame_callback;
    callbacks->redrawCb = redraw_callback;

    // Specific to the component 
    comp->pos = (screenpos) { .x = 2, .y = 1 };
    comp->lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    comp->style = term_style_make( COLOR_BRIGHT_BLACK, Properties_FAINT );

    return (struct ComponentHeader *)comp;
}
