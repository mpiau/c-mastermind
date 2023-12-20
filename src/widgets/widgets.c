#include "widgets/widgets.h"
#include "widgets/widget_definition.h"

#include "widgets/widget_timer.h"

enum // Constants
{
    // Should be enough spaces as I don't think we will have many duplicated widgets
    WIDGETS_POOL_SIZE = WidgetType_Count * 2
};

static struct Widget *s_widgetsPool[WIDGETS_POOL_SIZE] = {};
static u32 s_hookCount = 0;

static inline
bool is_full( void )
{
    return s_hookCount == WIDGETS_POOL_SIZE;
}


bool tryget_widget_index( struct Widget const *widget, u32 *outIndex )
{
    for ( u32 idx = 0; idx < s_hookCount; ++idx )
    {
        if ( s_widgetsPool[idx]->id == widget->id )
        {
            *outIndex = idx;
            return true;
        }
    }

    return false;
}

bool widgets_hook( struct Widget *widget )
{
    assert( widget );
    if ( is_full() ) return false;

    u32 index;
    if ( tryget_widget_index( widget, &index ) )
    {
        // ALready hooked !
        return false;
    }

    // Check if we simply can add it to the end without breaking the ascend order
    if ( s_widgetsPool[s_hookCount - 1]->order < widget->order )
    {
        s_widgetsPool[s_hookCount] = widget;
        widget->references += 1;
        s_hookCount += 1;
        return true;
    }

    // Otherwise, search for its rightful place, and shift the next ones by one.
    for ( u32 idx = 0; idx < s_hookCount; ++idx )
    {
        if ( s_widgetsPool[idx]->order < widget->id )
        {
            for ( u32 reverseIdx = s_hookCount; reverseIdx > idx; reverseIdx-- )
            {
                s_widgetsPool[reverseIdx - 1] = s_widgetsPool[reverseIdx];
            }

            s_widgetsPool[idx] = widget;
            widget->references += 1;
            s_hookCount += 1;
            return true;
        }
    }

    assert( false ); // shouldn't happen
    return false;
}

void widgets_unhook( struct Widget const *widget )
{
    assert( widget );

    u32 index;
    if ( !tryget_widget_index( widget, &index ) ) return;

    s_widgetsPool[index]->references -= 1;
    s_widgetsPool[index] = NULL;
    s_hookCount -= 1;

    // Replace the deleted one with the last hooked.
    if ( s_hookCount > 0 )
    {
        s_widgetsPool[index ] = s_widgetsPool[s_hookCount];
        s_widgetsPool[s_hookCount] = NULL;
    }
}

bool widgets_is_hook( struct Widget const *widget )
{
    u32 index;
    return tryget_widget_index( widget, &index );
}


// /////////////////////////////////////////////////////////


void widgets_frame( void )
{

}


void widgets_hide( void )
{
    for ( u32 idx = 0; idx < s_hookCount; ++idx )
    {
        struct Widget *widget = s_widgetsPool[idx];
        if ( widget && widget->hideCallback != NULL )
        {
            widget->hideCallback();
        }
    }
}