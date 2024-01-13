#include "components/component_framerate.h"

#include "components/component_header.h"
#include "fps_counter.h"
#include "terminal/terminal.h"

#include <stdlib.h>


struct ComponentFramerate
{
    struct ComponentHeader header;

    screenpos   pos;
    usize       lastAverageFPS;
    struct Style style;
};

#define CAST_TO_COMPONENT( _header ) ( ( struct ComponentFramerate * )( _header ) )

#define RETURN_IF_DISABLED( _header )                   \
    do { if ( !_header->enabled ) return; } while ( 0 )


static void on_refresh_callback( struct ComponentHeader const *header )
{
    RETURN_IF_DISABLED( header );

    struct ComponentFramerate const *comp = CAST_TO_COMPONENT( header );

    cursor_update_pos( comp->pos );
    style_update( comp->style );
    term_write( L"%3uFPS", comp->lastAverageFPS );
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
    callbacks->refreshCb = on_refresh_callback;

    // Specific to the component 
    comp->pos = (screenpos) { .x = 2, .y = 1 };
    comp->lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    comp->style = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );

    return (struct ComponentHeader *)comp;
}
