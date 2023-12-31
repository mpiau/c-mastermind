#include "widgets/widget_board.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "console.h"

struct WidgetBoard
{
	struct Widget header;
};


enum // Constants
{
	PEG_WIDTH = 6,
	PEG_INTERSPACE = 3,
	PIN_WIDTH = 4,
	PIN_INTERSPACE = 1,
	ROW_HEIGHT = 6,
};

void clear_callback( struct Widget *widget )
{
	widget_utils_clear_content( &widget->box );
}


static void draw_peg( screenpos const ul, enum ConsoleColorFG const color, bool const emptySpace )
{
	console_color_fg( color );

	console_cursor_set_position( ul.y, ul.x );
	console_draw( emptySpace ? L",:'':." : L",d||b." );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( emptySpace ? L":    :" : L"OOOOOO" );//, 2, emptySpace ? L"" : L"01" );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( emptySpace ? L"`:,,:'" : L"`Y||P'" );
}

static void draw_pin( screenpos ul, enum ConsoleColorFG color, bool const emptySpace )
{
	console_color_fg( color );

	console_cursor_set_position( ul.y, ul.x );
	console_draw( emptySpace ? L".''." : L",db." );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( emptySpace ? L"`,,'" : L"`YP'" );
}


static void draw_row_board( screenpos const ul, u16 const nbPegs, u16 const nbPins )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );

	u16 const nbPegSpaces = nbPegs * 9 - 3; // 9 = pegs(6) + interspace(3) | -3 because one less interspace than nb pegs
	u16 const nbPinSpaces = ( ( nbPins + 1 ) / 2 ) * 5 - 1; // pin(4) + interspace(1) | -1 because one less interspace

	console_cursor_set_position( ul.y, ul.x );
	console_draw( L"#################" );
	for ( u16 x = 0; x < nbPegSpaces + nbPinSpaces; ++x ) console_draw( L"#" );

	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L"### %*lc #####  %*lc  ###", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( L"##  %*lc  ###   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 3, ul.x );
	console_draw( L"##  %*lc  ###   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 4, ul.x );
	console_draw( L"##  %*lc  ###   %*lc   ##", nbPegSpaces, L' ', nbPinSpaces, L' ' );
	console_cursor_set_position( ul.y + 5, ul.x );
	console_draw( L"### %*lc #####  %*lc  ###", nbPegSpaces, L' ', nbPinSpaces, L' ' );

/*	console_cursor_set_position( ul.y + 6, ul.x );
	console_draw( L"#################" );
	for ( u16 x = 0; x < nbPegSpaces + nbPinSpaces; ++x ) console_draw( L"#" );*/
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
	if ( pins == NULL ) return;
	bool const oddNbPins = nbPins % 2 != 0; // Need to add a last empty pin manually

	u16 const tmp = ul.x + 12 + nbPegs * 9 - 3;
	draw_pin( (screenpos){ .x = tmp, .y = ul.y + 1}, pins[0].color, pins[0].isEmpty );
}


static void draw_row( screenpos const ul /* + game content to take pegs from */ )
{
	struct PegV2 pegs[6] = {};
	pegs[0] = (struct PegV2) { .color = ConsoleColorFG_MAGENTA, .isEmpty = false };
	pegs[1] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_RED, .isEmpty = false };
 	pegs[2] = (struct PegV2) { .color = ConsoleColorFG_YELLOW, .isEmpty = false };
	pegs[3] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_GREEN, .isEmpty = false };
	pegs[4] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_BLACK, .isEmpty = true };
	pegs[5] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_BLACK, .isEmpty = true };
	struct PegV2 pins[6] = {};
	pins[0] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_RED, .isEmpty = false };
	pins[1] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_WHITE, .isEmpty = false };
	pins[2] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_BLACK, .isEmpty = true };
	pins[3] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_BLACK, .isEmpty = true };
	pins[4] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_BLACK, .isEmpty = true };
	pins[5] = (struct PegV2) { .color = ConsoleColorFG_BRIGHT_BLACK, .isEmpty = true };
	draw_row_board( ul, ARR_COUNT( pegs ), ARR_COUNT( pins ) );

	draw_row_pegs( ul, pegs, ARR_COUNT( pegs ) );
	draw_row_pins( ul, ARR_COUNT( pegs ), pins, ARR_COUNT( pins ) );
	draw_row_turn( ul, ARR_COUNT( pegs ), 12 );
}


static void draw_header_board( screenpos const ul )
{
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );

	// TODO needs to adjust depending of the number of pegs

	console_cursor_set_position( ul.y, ul.x );
	console_draw( L"   ##############################################   " );
	console_cursor_set_position( ul.y + 1, ul.x );
	console_draw( L" ####                                          #### " );
	console_cursor_set_position( ul.y + 2, ul.x );
	console_draw( L" ##                                              ## " );
	console_cursor_set_position( ul.y + 3, ul.x );
	console_draw( L"###                                              ###" );
	console_cursor_set_position( ul.y + 4, ul.x );
	console_draw( L"#####                                          #####" );
	console_cursor_set_position( ul.y + 5, ul.x );
	console_draw( L"####################################################" );
	console_cursor_set_position( ul.y + 6, ul.x );
	console_draw( L"####################################################" );
}


static void draw_header_title( screenpos const ul, u16 const nbPegs )
{
}


static void redraw_callback( struct Widget *widget )
{
	int x = widget->box.contentUpLeft.x + 2;
	int y = widget->box.contentUpLeft.y + 1;

//	draw_header_board( (screenpos){ x, y } );
//	draw_header_title( (screenpos){ x, y }, 3 );
/*	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
	console_cursor_set_position( y++, x );
	console_draw( L"   ##############################################   " );
	console_cursor_set_position( y++, x );
	console_draw( L" #### %S_  _ ___ ___ ___ ___ ___ _  _ _ _  _ __  %S#### ", L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L" ##   %S|\\/| |_| [_   |  |_  |_/ |\\/| | |\\ | | \\   %S## ",  L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L"###   %S|  | | | __]  |  |__ | \\ |  | | | \\| |_/   %S###",  L"\x1b[1;33m", L"\x1b[0;94m" );
	console_cursor_set_position( y++, x );
	console_draw( L"#####                                          #####" );
	console_cursor_set_position( y++, x );
	console_draw( L"####################################################" );
	console_draw( L"####################################################" );
	draw_row( (screenpos){ .x = x, .y = y } );*/
/*	console_cursor_set_position( y++, x );
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
*/
	draw_row( (screenpos){ .x = x, .y = y } );
	draw_row( (screenpos){ .x = x, .y = y + ROW_HEIGHT } );
	draw_row( (screenpos){ .x = x, .y = y + ROW_HEIGHT * 2 } );

	y = y + ROW_HEIGHT * 3;
	x = x - 2;
	console_cursor_set_position( y, widget->box.contentUpLeft.x + 2 );
	console_color_fg( ConsoleColorFG_BRIGHT_BLUE );
	console_draw( L"##################################################################################" );
//	for ( u16 x = 0; x < nbPegSpaces + nbPinSpaces; ++x ) console_draw( L"#" );

	y += 1;

	console_cursor_set_position( y + 1, widget->box.contentUpLeft.x );
	console_color_fg( ConsoleColorFG_WHITE );
	console_draw( L"──────────────────────────────────────────────────────────────────────────────────────" );

	y += 1;

	x += 8;

	draw_peg( (screenpos){ .x = x, .y = y + 1 }, ConsoleColorFG_MAGENTA, false );
	draw_peg( (screenpos){ .x = x + 1 + 8, .y = y + 1 }, ConsoleColorFG_BRIGHT_RED, false );
	draw_peg( (screenpos){ .x = x + 2 + 16, .y = y + 1 }, ConsoleColorFG_YELLOW, false );
	draw_peg( (screenpos){ .x = x + 3 + 24, .y = y + 1 }, ConsoleColorFG_BRIGHT_GREEN, false );
	draw_peg( (screenpos){ .x = x + 4 + 32, .y = y + 1 }, ConsoleColorFG_WHITE, false );
	draw_peg( (screenpos){ .x = x + 5 + 40, .y = y + 1 }, ConsoleColorFG_BRIGHT_CYAN, false );
	draw_peg( (screenpos){ .x = x + 6 + 48, .y = y + 1 }, ConsoleColorFG_BRIGHT_BLUE, false );
	draw_peg( (screenpos){ .x = x + 7 + 56, .y = y + 1 }, ConsoleColorFG_BRIGHT_YELLOW, false );
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
    vec2u16 const contentSize  = (vec2u16)   { .x = 86, .y = 25 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;
	widget_utils_set_title( &widget->box, L"Mastermind", ConsoleColorFG_BRIGHT_GREEN );

	widget->callbacks.redrawCb = redraw_callback;
	widget->callbacks.frameCb = frame_callback;
	widget->callbacks.clearCb = clear_callback;

	return (struct Widget *)board;
}
