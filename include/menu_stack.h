#pragma once

#include "game_menus.h"

enum
{
	MENU_STACK_MAX_DEPTH     = 10
};


struct MenuStack
{
	struct GameMenu *stack[MENU_STACK_MAX_DEPTH];
	usize count;
};


void mstack_reset( struct MenuStack *menuStack );
void mstack_init( struct MenuStack *menuStack );

bool mstack_is_full( struct MenuStack *menuStack );
bool mstack_is_empty( struct MenuStack *menuStack );

bool mstack_push( struct MenuStack *menuStack, struct GameMenu *menu );
bool mstack_pop( struct MenuStack *menuStack, struct GameMenu **menu );

struct GameMenu *mstack_try_peek( struct MenuStack *menuStack );

usize mstack_size( struct MenuStack *menuStack );
usize mstack_capacity( struct MenuStack *menuStack );
