#include "core_types.h"
#include "terminal.h"

/*enum WidgetId
{
	WidgetId_TEXT,
	WidgetId_SELECT,
	WidgetId_SELECTABLE_TEXT,
};*/


struct WidgetText
{
	char *text;
	enum TermColor color;
};


struct WidgetSelect
{
	char *prefix;
	char *suffix;
	enum TermColor color; // TERM_COLOR_NONE to not override the color.
};


struct WidgetSelectableText
{
	struct WidgetText widgetText;
	struct WidgetSelect widgetSelect;
	bool selected;
};


struct WidgetVertSelectMenu
{
	struct WidgetText title;
	struct WidgetSelectableText *rows;
	usize rowsCount;
	usize rowsCapacity;
	usize selectedRowIdx;
};

/*
void widget_text_init( struct WidgetText *widget, char *text, enum TermColor color );
void widget_select_init( struct WidgetSelect *widget, char *prefix, char *suffix, enum TermColor color );
void widget_selectable_text_init( struct WidgetSelectableText *widget, struct WidgetText *text, struct WidgetSelect *select );

bool widget_vertselect_menu_create( struct WidgetVertSelectMenu *widget, struct WidgetText *title, usize capacity );
bool widget_vertselect_menu_add_row( struct WidgetVertSelectMenu *widget, struct WidgetSelectableText *row );
*/