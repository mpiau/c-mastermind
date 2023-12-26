#include "widgets/widget_board.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "console.h"

struct WidgetBoard
{
	struct Widget header;
};


void clear_callback( struct Widget *widget )
{
	widget_utils_clear_content( &widget->box );
}

void redraw_callback( struct Widget *widget )
{
	int x = widget->box.contentUpLeft.x + 4;
	int y = widget->box.contentUpLeft.y + 2;

	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );//BRIGHT_BLACK );
	console_cursor_set_position( y++, x );
	console_draw( L"   #######################################################   " );
	console_cursor_set_position( y++, x );
	console_draw( L" ####   %S_  _ ____ ____ ___ ____ ____ _  _ _ _  _ ___    %S#### ",
		L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L" ##     %S|\\/| |__| [__   |  |___ |__/ |\\/| | |\\ | |  \\     %S## ",  L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L"###     %S|  | |  | ___]  |  |___ |  \\ |  | | | \\| |__/     %S###",  L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L"#####                                                   #####" );
	console_cursor_set_position( y++, x );
	console_draw( L"#############################################################" );
	console_cursor_set_position( y++, x );
	console_draw( L"#############################################################" );
	console_cursor_set_position( y++, x );
	console_draw( L"###                                   #####  %S,db. %S,db.  %S#####", 
		L"\x1b[1;31m", L"\x1b[1;31m", L"\x1b[0;94m"
	);
	console_cursor_set_position( y++, x );
	console_draw( L"##  %S,d88b.   %S,d88b.   %S.::::.   %S,d88b.  %S###   %S`YP' %S`YP'   %S####",
		L"\x1b[1;31m", L"\x1b[32m", L"\x1b[90m", L"\x1b[33m", L"\x1b[94m",
		L"\x1b[31m", L"\x1b[31m", L"\x1b[94m"
	 );
	console_cursor_set_position( y++, x );
	console_draw( L"##  %S888888   %S888888   %S::::::   %S888888  %S### %S01%S            ####",
		L"\x1b[31m", L"\x1b[32m", L"\x1b[90m", L"\x1b[33m", L"\x1b[94m",
		L"\x1b[37m", L"\x1b[94m"
	);
	console_cursor_set_position( y++, x );
	console_draw( L"##  %S`Y88P'   %S`Y88P'   %S`::::'   %S`Y88P'  %S###   %S,db. %S.::.%S   ####", 
		L"\x1b[31m", L"\x1b[32m", L"\x1b[90m", L"\x1b[33m", L"\x1b[94m",
		L"\x1b[37m", L"\x1b[90m", L"\x1b[94m"
	);
	console_cursor_set_position( y++, x );
	console_draw( L"###                                   #####  %S`YP' %S'::'  %S#####",
		L"\x1b[37m", L"\x1b[90m", L"\x1b[94m"
	);
	console_cursor_set_position( y++, x );
	console_draw( L"#############################################################" );
	console_cursor_set_position( y++, x );
	console_draw( L"#############################################################" );
}


// Shouldn't need a frame callback as it depends on user input only
static void frame_callback( struct Widget *widget )
{
	static bool h = false;
	if ( !h )
	{
		redraw_callback( widget );
		h = true;
	}
}

struct Widget *widget_board_create( void )
{
    struct WidgetBoard *const board = malloc( sizeof( struct WidgetBoard ) );
    if ( !board ) return NULL;

	struct Widget *const widget = &board->header;

    widget->id = WidgetId_BOARD;
	widget->visibilityStatus = WidgetVisibilityStatus_VISIBLE;

    screenpos const borderUpLeft = (screenpos) { .x = 2, .y = 3 };
    vec2u16 const contentSize  = (vec2u16)   { .x = 71, .y = 25 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;
	widget_utils_set_title( &widget->box, L"Mastermind", ConsoleColorFG_BRIGHT_GREEN );

	widget->callbacks.redrawCb = redraw_callback;
	widget->callbacks.frameCb = frame_callback;
	widget->callbacks.clearCb = clear_callback;

	return (struct Widget *)board;
}
