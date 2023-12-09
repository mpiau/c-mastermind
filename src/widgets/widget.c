#include "widgets/widget.h"

#include <stdio.h>

/*
void widget_text_init( struct WidgetText *widget, char const *text, enum TermColor const color )
{
    widget->text = text;
    widget->color = color;
}

void widget_select_init( struct WidgetSelect *widget, char *prefix, char *suffix, enum TermColor const color )
{
    widget->color = color;
    widget->prefix = prefix;
    widget->suffix = suffix;
}

void widget_selectable_text_init( struct WidgetSelectableText *widget, struct WidgetText const *text, struct WidgetSelect const *select )
{
    widget->widgetText = *text;
    widget->widgetSelect = *select;
    widget->selected = false;
}



bool widget_vertselect_menu_create( struct WidgetVertSelectMenu *widget, struct WidgetText const *title, usize capacity )
{
    usize const allocSize = sizeof( *widget->rows ) * capacity;
    widget->rows = malloc( allocSize ); // TODO shutdown function to free the resources.
    if ( !widget->rows )
    {
        fprintf( stderr, "widget_vertselect_menu_create: Allocation failed (size %u)\n", allocSize );
        return false;
    }

    widget->title = *title;
    widget->rowsCount = 0;
    widget->rowsCapacity = capacity;
    widget->selectedRowIdx = 0;

    return true;
}

bool widget_vertselect_menu_add_row( struct WidgetVertSelectMenu *widget, struct WidgetSelectableText const *row )
{
    if ( widget->rowsCount == widget->rowsCapacity )
    {
        fprintf( stderr, "widget_vertselect_menu_add_row: Maximum capacity reached (%u)\n", widget->rowsCapacity );
        return false;        
    }

    widget->rows[widget->rowsCount] = *row;
    widget->rowsCount += 1;

    return true;
}
*/