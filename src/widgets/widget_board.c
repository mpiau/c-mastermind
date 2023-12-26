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


static void draw_peg( screenpos const ul, enum ConsoleColorFG const color, bool const emptySpace )
{
	console_color_fg( color );

	console_cursor_set_position( ul.y, ul.x );
	console_draw( emptySpace ? L",:``:." : L",d||b." );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( emptySpace ? L":    :" : L"OOOOOO" );//, 2, emptySpace ? L"" : L"01" );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( emptySpace ? L"`:,,:'" : L"`Y||P'" );
}

static void draw_pin( screenpos ul, enum ConsoleColorFG color, bool const emptySpace )
{
	console_color_fg( color );

	console_cursor_set_position( ul.y, ul.x );
	console_draw( emptySpace ? L".``." : L",db." );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( emptySpace ? L"`,,'" : L"`YP'" );
}


static void draw_row_board( screenpos const ul, u16 const nbPegs, u16 const nbPins )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );

	u16 const nbPegSpaces = nbPegs * 9 - 3; // 9 = pegs(6) + interspace(3) | -3 because one less interspace than nb pegs
	u16 const nbPinSpaces = ( ( nbPins + 1 ) / 2 ) * 5 - 1; // pin(4) + interspace(1) | -1 because one less interspace

	console_cursor_set_position( ul.y, ul.x );
	console_draw( L"#############################################################" );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L"### %*lc #####  %*lc  #####", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( L"##  %*lc  ###   %*lc   ####", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 3, ul.x );
	console_draw( L"##  %*lc  ###   %*lc   ####", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 4, ul.x );
	console_draw( L"##  %*lc  ###   %*lc   ####", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 5, ul.x );
	console_draw( L"### %*lc #####  %*lc  #####", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 6, ul.x );
	console_draw( L"#############################################################" );
}


static void draw_row_turn( screenpos const ul, u16 const nbPegs, u32 const turn )
{
	console_color_fg( ConsoleColorFG_WHITE );

	console_cursor_set_position( ul.y + 3, ul.x + 10 + ( nbPegs * 9 ) - 3 );
	console_draw( L"%02u", turn );
}


struct PegV2
{
	enum ConsoleColorFG color;
	bool isEmpty;
};

static void draw_row_pegs( screenpos const ul, struct PegV2 const *pegs, u32 const nbPegs )
{
	for ( u32 pegIdx = 0; pegIdx < nbPegs; ++pegIdx )
	{
		screenpos const pegUL = (screenpos) {
			.x = ul.x + 4 + ( pegIdx * 9 ),
			.y = ul.y + 2
		};
		draw_peg( pegUL, pegs[pegIdx].color, pegs[pegIdx].isEmpty );
	}
}


static void draw_row_pins( screenpos const ul, u32 const nbPegs, struct PegV2 const *pins, u32 const nbPins )
{
	bool const oddNbPins = nbPins % 2 != 0; // Need to add a last empty pin manually

	u16 const tmp = ul.x + 12 + nbPegs * 9 - 3;
	draw_pin( (screenpos){ .x = tmp, .y = ul.y + 1}, pins[0].color, pins[0].isEmpty );
}


static void draw_row( screenpos const ul /* + game content to take pegs from */ )
{
	struct PegV2 pegs[3] = {};
	pegs[0] = (struct PegV2) { .color = ConsoleColorFG_MAGENTA, .isEmpty = false };
	pegs[1] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_RED, .isEmpty = false };
// 	pegs[2] = (struct PegV2) { .color = ConsoleColorFG_YELLOW, .isEmpty = false };
//	pegs[3] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_GREEN, .isEmpty = false };
	struct PegV2 pins[3] = {};
	pins[0] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_RED, .isEmpty = false };
	pins[1] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_WHITE, .isEmpty = false };
	pins[2] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_BLACK, .isEmpty = true };

	draw_row_board( ul, ARR_COUNT( pegs ), ARR_COUNT( pins ) );

	draw_row_pegs( ul, pegs, ARR_COUNT( pegs ) );
	draw_row_pins( ul, ARR_COUNT( pegs ), pins, ARR_COUNT( pins ) );
	draw_row_turn( ul, ARR_COUNT( pegs ), 12 );
}


void redraw_callback( struct Widget *widget )
{
	int x = widget->box.contentUpLeft.x + 5;
	int y = widget->box.contentUpLeft.y + 1;

	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
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
	console_draw( L"##  %S,d||b.   %S,d88b.   %S,:!!:.   %S,d88b.  %S###   %S`YP' %S`YP'   %S####",
		L"\x1b[1;31m", L"\x1b[32m", L"\x1b[90m", L"\x1b[33m", L"\x1b[94m",
		L"\x1b[31m", L"\x1b[31m", L"\x1b[94m"
	 );
	console_cursor_set_position( y++, x );
	console_draw( L"##  %S888888   %S888888   %S8::::8   %S888888  %S### %S01%S            ####",
		L"\x1b[31m", L"\x1b[32m", L"\x1b[90m", L"\x1b[33m", L"\x1b[94m",
		L"\x1b[37m", L"\x1b[94m"
	);
	console_cursor_set_position( y++, x );
	console_draw( L"##  %S`Y||P'   %S`Y88P'   %S`:;;:'   %S`Y88P'  %S###   %S,db. %S.::.%S   ####", 
		L"\x1b[31m", L"\x1b[32m", L"\x1b[90m", L"\x1b[33m", L"\x1b[94m",
		L"\x1b[37m", L"\x1b[90m", L"\x1b[94m"
	);
	console_cursor_set_position( y++, x );
	console_draw( L"###                                   #####  %S`YP' %S'::'  %S#####",
		L"\x1b[37m", L"\x1b[90m", L"\x1b[94m"
	);
	console_cursor_set_position( y++, x );
	console_draw( L"#############################################################" );

	draw_row( (screenpos){ .x = x, .y = y } );
//	y += 1;
//	draw_peg( (screenpos){ .x = x, .y = y }, ConsoleColorFG_BRIGHT_YELLOW, false );
//	x += 9;
//	draw_peg( (screenpos){ .x = x, .y = y }, ConsoleColorFG_BRIGHT_BLACK, true );
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
