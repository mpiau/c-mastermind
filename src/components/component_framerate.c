#include "components/component_framerate.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

#include "fps_counter.h"
#include "mouse.h"

#include "console.h"

struct ComponentFramerate
{
    struct Widget header;

    screenpos   pos;
    usize       lastAverageFPS;
    struct Attr attributes;
};
#define CAST_TO_COMPONENT( _header ) ( struct ComponentFramerate * )( _header )

#define RETURN_IF_DISABLED( _header )                   \
    do { if ( !_header->enabled ) return; } while ( 0 )


static void redraw_callback( struct Widget *header )
{
    RETURN_IF_DISABLED( header );

    struct ComponentFramerate const *comp = CAST_TO_COMPONENT( header );

    console_set_cpos( comp->pos );
    console_set_attr( comp->attributes ); 
    console_write( L"%3uFPS", comp->lastAverageFPS );
}


static void frame_callback( struct Widget *header )
{
    RETURN_IF_DISABLED( header );

    struct ComponentFramerate *comp = CAST_TO_COMPONENT( header );

    usize const lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    if ( lastAverageFPS != comp->lastAverageFPS )
    {
        comp->lastAverageFPS = lastAverageFPS;
        header->redrawNeeded = true;
    }
}


struct Widget *component_framerate_create( void )
{
    struct ComponentFramerate *const comp = calloc( 1, sizeof( struct ComponentFramerate ) );
    if ( !comp ) return NULL;

    widget_set_header( &comp->header, ComponentId_FRAMERATE, true );

    struct WidgetCallbacks *const callbacks = &comp->header.callbacks;
    callbacks->frameCb = frame_callback;
    callbacks->redrawCb = redraw_callback;

    // Specific to the component 
    comp->pos = SCREENPOS( 2, 1 );
    comp->lastAverageFPS = fpscounter_average_framerate( fpscounter_get_instance() );
    comp->attributes = ATTR( AttrColor_BLACK_FG, AttrStyle_DEFAULT, AttrShade_BRIGHT );

    return (struct Widget *)comp;
}
