#include "ui/widgets.h"

#include "terminal/terminal.h"
#include "rect.h"

#include <stdlib.h>


struct WidgetBuildVersion
{
    struct Widget base;

    struct Rect rect;
};


static void enable_callback( struct Widget *base )
{
    struct WidgetBuildVersion *widget = (struct WidgetBuildVersion *)base;

    cursor_update_pos( rect_get_ul_corner( &widget->rect ) );
    style_update( STYLE_WITH_ATTR( FGColor_BRIGHT_BLACK, Attr_FAINT | Attr_ITALIC ) );
    term_write( L"Development Build" );
}


static void disable_callback( struct Widget *base )
{
    struct WidgetBuildVersion *widget = (struct WidgetBuildVersion *)base;

    rect_clear( &widget->rect );
}


struct Widget *widget_buildversion_create( void )
{
    struct WidgetBuildVersion *const widget = calloc( 1, sizeof( struct WidgetBuildVersion ) );
    if ( !widget ) return NULL;

    widget->base.name = "BuildVersion";
    widget->base.enabledScenes = UIScene_ALL;
    widget->base.enableCb = enable_callback;
    widget->base.disableCb = disable_callback;

    // Widget specific

    widget->rect = rect_make( SCREENPOS( 1, 30 ), VEC2U16( 20, 1 ) );

    return (struct Widget *)widget;
}
