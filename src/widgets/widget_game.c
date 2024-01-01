#include "widgets/widget_game.h"

#include "widgets/widget_definition.h"
#include "widgets/widget_utils.h"

struct WidgetGame
{
	struct Widget header;
};


struct Widget *widget_game_create( void )
{
    struct WidgetGame *const game = malloc( sizeof( struct WidgetGame ) );
    if ( !game ) return NULL;

	struct Widget *const widget = &game->header;

    widget->id = WidgetId_GAME;
	widget->enabled = true;

    screenpos const borderUpLeft = (screenpos) { .x = 1, .y = 1 };
    screenpos const contentSize  = (vec2u16)   { .x = 118, .y = 28 };
	widget_utils_set_position( &widget->box, borderUpLeft, contentSize );
	widget->box.borderOption = WidgetBorderOption_INVISIBLE;

	return (struct Widget *)game;
}
