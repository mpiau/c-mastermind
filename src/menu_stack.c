#include "menu_stack.h"

#include <unistd.h>


void mstack_reset( struct MenuStack *const menuStack )
{
    memset( menuStack->stack, 0, sizeof( menuStack->stack ) );
    menuStack->count = 0;
}


void mstack_init( struct MenuStack *const menuStack )
{
    mstack_reset( menuStack );
}


bool mstack_is_full( struct MenuStack *const menuStack )
{
    return menuStack->count >= ARR_COUNT( menuStack->stack );
}


bool mstack_is_empty( struct MenuStack *const menuStack )
{
    return menuStack->count == 0;
}


bool mstack_push( struct MenuStack *const menuStack, struct GameMenu *const menu )
{
    if ( mstack_is_full( menuStack ) ) return false;

    menuStack->stack[menuStack->count] = menu;
    menuStack->count += 1;
    return true;
}


bool mstack_pop( struct MenuStack *const menuStack, struct GameMenu * *const menu )
{
    if ( mstack_is_empty( menuStack ) ) return false;

    menuStack->count -= 1;
    *menu = menuStack->stack[menuStack->count];
    return true;
}


struct GameMenu *mstack_try_peek( struct MenuStack *const menuStack )
{
    if ( mstack_is_empty( menuStack ) ) return NULL;

    return menuStack->stack[menuStack->count - 1];
}


usize mstack_size( struct MenuStack *const menuStack )
{
    return menuStack->count;
}


usize mstack_capacity( struct MenuStack *const menuStack )
{
    return ARR_COUNT( menuStack->stack );
}
