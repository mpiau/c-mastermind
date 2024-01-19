#include "components/component_framerate.h"

#include "components/component_header.h"
#include "fps_counter.h"
#include "terminal/terminal.h"
#include "rect.h"

#include <stdlib.h>


struct ComponentFramerate
{
    struct ComponentHeader header;

    struct Rect rect;
    struct Style style;
    usize lastAverageFPS;
};

#define CAST_TO_COMP( _header ) ( ( struct ComponentFramerate * )( _header ) )


static void frame_callback( struct ComponentHeader *header )
{
    struct ComponentFramerate *comp = CAST_TO_COMP( header );

    usize const lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    if ( lastAverageFPS != comp->lastAverageFPS )
    {
        comp->lastAverageFPS = lastAverageFPS;
        cursor_update_pos( rect_get_ul_corner( &comp->rect ) );
        style_update( comp->style );
        term_write( L"%3uFPS", comp->lastAverageFPS );
    }
}


static void enable_callback( struct ComponentHeader *header )
{
    struct ComponentFramerate *comp = CAST_TO_COMP( header );
    comp->lastAverageFPS = 0;
}


static void disable_callback( struct ComponentHeader *header )
{
    struct ComponentFramerate *comp = CAST_TO_COMP( header );
    rect_clear( &comp->rect );
}


struct ComponentHeader *component_framerate_create( void )
{
    struct ComponentFramerate *const comp = calloc( 1, sizeof( struct ComponentFramerate ) );
    if ( !comp ) return NULL;

    comp->header.id = ComponentId_FRAMERATE;

    struct ComponentCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->enableCb = enable_callback;
    callbacks->disableCb = disable_callback;
    callbacks->frameCb = frame_callback;

    comp->rect = rect_make( SCREENPOS( 1, 1 ), VEC2U16( 7, 1 ) );
    comp->style = STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT );

    return (struct ComponentHeader *)comp;
}
