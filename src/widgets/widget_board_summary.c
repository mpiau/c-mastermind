#include "widgets/widget_board_summary.h"

#include "mastermind_v2.h"
#include "widgets/widget_definition.h"

struct WidgetBoardSummary
{
    struct Widget header;

    // Specific to the widget
    struct MastermindV2 game;
};

static struct WidgetBoardSummary s_boardSummary = {};
static bool s_init = false;

// TODO - FIXME : Only give the board, as only the board should be useful
//                to the widget here.

bool widget_board_summary_init( struct MastermindV2 const *const mastermind )
{
    if ( s_init ) { return false; }
    if ( !mastermind ) { return false; }

    s_boardSummary.header.id = WidgetId_BOARD_SUMMARY;
    // [...]

    widgets_hook( (struct Widget *)&s_boardSummary );

    s_init = true;
    return s_init;
}


void widget_board_summary_uninit( void )
{
    if ( s_init )
    {
        widgets_unhook( (struct Widget *)&s_boardSummary );
        s_init = false;
    }
}
