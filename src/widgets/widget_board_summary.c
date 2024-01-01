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
    u32 const nbTurns = 15;
    u32 const nbPegsPerTurn = 6;
    // 4 -> border + space each side, + 2 -> middle with - and space
    u32 const borderWidth = 4 + 3 * nbPegsPerTurn + 2;

	screenpos pos = widget->box.contentUpLeft;    

    for ( int y = 0; y < nbTurns; ++y )
    {
        for ( int x = 0; x < nbPegsPerTurn; ++x )
        {
			console_cursor_set_position( pos.y + y, pos.x + ( x * 2 ) );
			console_draw( L" %lc", UTF16C_BigFilledCircle );
        }
		console_draw( L" - " );
        for ( int x = 0; x < nbPegsPerTurn; ++x )
        {
			console_draw( L"%lc", UTF16C_SmallDottedCircle );
        }
    }

	pos.y += nbTurns;
	console_cursor_set_position( pos.y, pos.x );
	console_draw( L"MASTERMIND" );
	pos.y += 1;
    for ( int x = 0; x < nbPegsPerTurn; ++x )
    {
		console_cursor_set_position( pos.y, pos.x + ( x * 2 ) );
		console_draw( L" ?", UTF16C_BigFilledCircle );
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


struct Widget *widget_board_summary_create( void )
{
    struct WidgetBoardSummary *const boardSummary = malloc( sizeof( struct WidgetBoardSummary ) );
    if ( !boardSummary ) return NULL;

	struct Widget *const widget = &boardSummary->header;

    widget->id = WidgetId_BOARD_SUMMARY;
	widget->enabled = true;

	assert( widget_exists( WidgetId_TIMER ) );
	struct WidgetBox const *timerBox = &widget_optget( WidgetId_TIMER )->box;

    screenpos const borderUpLeft = (screenpos) { .x = timerBox->borderUpLeft.x, .y = timerBox->borderBottomRight.y + 1 };
    screenpos const contentSize  = (vec2u16)   { .x = 22, .y = 22 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_ALWAYS_VISIBLE;
	widget_utils_set_title( &widget->box, L"Board Summary", ConsoleColorFG_YELLOW );

    struct WidgetCallbacks *const callbacks = &widget->callbacks;
//    callbacks->frameCb = frame_callback;
    callbacks->redrawCb = redraw_callback;

	return (struct Widget *)boardSummary;
}
