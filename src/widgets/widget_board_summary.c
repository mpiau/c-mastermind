#include "widgets/widget_board_summary.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"
#include "characters_list.h"


struct WidgetBoardSummary
{
	struct Widget header;
};


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


static void redraw_callback( struct Widget *widget )
{
    u32 const nbTurns = 16;
    u32 const nbPegsPerTurn = 6;
	u16 const currTurn = 5;
    // 4 -> border + space each side, + 2 -> middle with - and space
    u32 const borderWidth = 4 + 3 * nbPegsPerTurn + 2;

	screenpos pos = widget->box.contentUpLeft;

	console_color_fg( ConsoleColorFG_BRIGHT_BLACK );

    for ( int y = 0; y < nbTurns; ++y )
    {
        for ( int x = 0; x < nbPegsPerTurn; ++x )
        {
			console_cursor_set_position( pos.y + y, pos.x + ( x * 2 ) );
			console_draw( L" %lc", UTF16C_BigFilledCircle );
        }
		console_draw( L" - " );
		if ( y + 1 == currTurn ) { console_draw( L"\x1B[2m" ); }
        for ( int x = 0; x < nbPegsPerTurn; ++x )
        {
			console_draw( L"%lc", UTF16C_SmallDottedCircle );
        }
    }

	pos.y += nbTurns;
	console_cursor_set_position( pos.y, pos.x );
	console_draw( L"     MASTERMIND" );
	pos.y += 1;
    for ( int x = 0; x < nbPegsPerTurn; ++x )
    {
		console_cursor_set_position( pos.y, pos.x + ( x * 2 ) );
		console_draw( L" ?" );
    }
	pos.y += 2;
	console_cursor_set_position( pos.y, pos.x );
	console_draw( L"SELECTED PEG" );
	pos.y += 1;
	pos.x += 7;
	draw_peg( pos, ConsoleColorFG_BRIGHT_MAGENTA, false );
	

/*    console_cursor_set_position( pos.y + config->nbTurns + 1, pos.x );

    u16 feedbackSpace = config->nbCodePegPerTurn + ( config->nbCodePegPerTurn - 1 ) * 2; // peg + 2 spaces between them
    u16 nbSpacesEachSide = ( borderWidth - feedbackSpace - 2 ) / 2; // - 2 because of the += 2 in the x at the beginning
    console_cursor_move_right_by( nbSpacesEachSide - 1 );

    for ( int x = 0; x < config->nbCodePegPerTurn; ++x )
    {
		board_draw_pegs( &board->solution[x], mastermind->board.hideSolution );
		console_cursor_move_right_by( 2 );
    }*/
}


static void on_game_update_callback( struct Widget *widget, struct Mastermind const *mastermind, enum GameUpdateType type )
{
	// TODO
}


struct Widget *widget_board_summary_create( void )
{
    struct WidgetBoardSummary *const boardSummary = malloc( sizeof( struct WidgetBoardSummary ) );
    if ( !boardSummary ) return NULL;
	memset( boardSummary, 0, sizeof( *boardSummary ) );

	struct Widget *const widget = &boardSummary->header;

    widget->id = WidgetId_BOARD_SUMMARY;
	widget->enabled = true;
	widget->redrawNeeded = true;

    screenpos const borderUpLeft = (screenpos) { .x = 95, .y = 3 };
    screenpos const contentSize  = (vec2u16)   { .x = 22, .y = 22 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;
	widget_utils_set_title( &widget->box, L"Game Summary", ConsoleColorFG_YELLOW );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
	callbacks->resizeCb = NULL;
	callbacks->frameCb = NULL;
    callbacks->redrawCb = redraw_callback;
	callbacks->gameUpdateCb = on_game_update_callback;

	return (struct Widget *)boardSummary;
}
