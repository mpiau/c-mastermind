#include "game_menus.h"

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <fcntl.h>


static void init_main_menu( struct GameMenu *const menu )
{
	menu->id = GameMenuId_Main;
	menu->type = GameMenuTypeVertSelect;

	static_assert( ARR_COUNT( menu->rows ) >= 3 );
	strcpy( menu->rows[0], "Single-player" );
    strcpy( menu->rows[1], "Local multiplayer (turn-based)" );
    strcpy( menu->rows[2], "Quit" );
	strcpy( menu->rowSeparator, "\n" );
	menu->nbRows = 3;
	menu->currSelectedRow = 0;

	static_assert( ARR_COUNT( menu->lineColors ) >= 2 );
	menu->lineColors[GameMenuRowStatus_DEFAULT] = TERM_WHITE;
	menu->lineColors[GameMenuRowStatus_SELECTED] = TERM_BOLD_GREEN;

	menu->hasPrefix = true;
	static_assert( ARR_COUNT( menu->prefixes ) >= 2 );
	strcpy( menu->prefixes[GameMenuRowStatus_DEFAULT], "  " );
	strcpy( menu->prefixes[GameMenuRowStatus_SELECTED], "> " );
}


bool game_menu_init( struct GameMenuList *const menus )
{
	init_main_menu( &menus->menus[GameMenuId_Main] );
}


void game_menu_shutdown( struct GameMenuList *const menus )
{
	// Nothing to do.
}


void game_menu_clear( struct GameMenu *const menu )
{
	for ( usize i = 0; i < menu->nbRows; ++i )
	{
		printf( "\033[1A\r\033[J" );
	}
	printf( "\033[1A\r\033[J" ); // TODO Temp for the title, to add to the structure
}


void game_menu_print( struct GameMenu *const menu )
{
	term_print( TERM_YELLOW, "MAIN MENU\n" ); // <- TODO Temp, to add to the structure
	for ( usize i = 0; i < menu->nbRows; ++i )
	{
		enum GameMenuRowStatus const status = ( i == menu->currSelectedRow )
			? GameMenuRowStatus_SELECTED
			: GameMenuRowStatus_DEFAULT;

		char const *const prefix = menu->hasPrefix ? menu->prefixes[status] : "" ;

		// TODO Allow specifics colors for the prefix / separator ?
		// TODO We may also want a suffix ?
		printf( "%s%s%s%s%s",
			S_COLOR_STR[menu->lineColors[status]],
			prefix,
			menu->rows[i],
			S_COLOR_STR_RESET,
			menu->rowSeparator
		);
	}
}


enum GameMenuId game_menu_enter( struct GameMenu *const menu )
{
	game_menu_print( menu );

	u8 oldSelect = menu->currSelectedRow;
	DWORD cNumRead;
	INPUT_RECORD irInBuf;
	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	bool stayInMenu = true;
	while ( stayInMenu && ReadConsoleInput( hStdin, &irInBuf, 1, &cNumRead ) )
	{
		if ( menu->currSelectedRow != oldSelect )
		{
			game_menu_print( menu );
			oldSelect = menu->currSelectedRow;
		}

		if ( irInBuf.EventType != KEY_EVENT ) continue;
		if ( !irInBuf.Event.KeyEvent.bKeyDown ) continue;

		if ( irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_UP && menu->currSelectedRow > 0 )
		{
			menu->currSelectedRow -= 1;
			game_menu_clear( menu );
		}
		else if ( irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_DOWN && menu->currSelectedRow < menu->nbRows - 1 )
		{
			menu->currSelectedRow += 1;
			game_menu_clear( menu );
		}
		else if ( irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_RETURN )
		{
			stayInMenu = false;
//			clear_main_menu( mainMenu ); // Do we keep that ?
		}
	}

	return GameMenuId_Quit;
}


void game_menu_loop( struct GameMenuList *const menus )
{
	while ( menus->current != GameMenuId_Quit )
	{
		menus->current = game_menu_enter( &menus->menus[menus->current] );
	}
}