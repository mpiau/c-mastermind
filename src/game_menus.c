#include "game_menus.h"
#include "keyboard_inputs.h"

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <fcntl.h>


static void init_main_menu( struct GameMenu *const menu )
{
	menu->id = GameMenuId_MAIN;
	menu->type = GameMenuTypeVertSelect;

	strcpy( menu->title, "MAIN MENU" );
	menu->titleColor = TERM_YELLOW;

	static_assert( ARR_COUNT( menu->rows ) >= 4 );
	strcpy( menu->rows[0], "Single-player" );
    strcpy( menu->rows[1], "Local multiplayer (turn-based)" );
    strcpy( menu->rows[2], "Game Settings" );
    strcpy( menu->rows[3], "Quit" );
	strcpy( menu->rowSeparator, "\n" );
	menu->nbRows = 4;
	menu->currSelectedRow = 0;

	static_assert( ARR_COUNT( menu->lineColors ) >= 2 );
	menu->lineColors[GameMenuStatus_DEFAULT] = TERM_WHITE;
	menu->lineColors[GameMenuStatus_FOCUS] = TERM_BOLD_GREEN;

	menu->hasPrefix = true;
	static_assert( ARR_COUNT( menu->prefixes ) >= 2 );
	strcpy( menu->prefixes[GameMenuStatus_DEFAULT], "  " );
	strcpy( menu->prefixes[GameMenuStatus_FOCUS], "> " );

	menu->hasSuffix = true;
	static_assert( ARR_COUNT( menu->suffixes ) >= 2 );
	strcpy( menu->suffixes[GameMenuStatus_DEFAULT], "" );
	strcpy( menu->suffixes[GameMenuStatus_FOCUS], " <" );
}


static void init_menu_settings( struct GameMenu *const menu )
{
	menu->id = GameMenuId_Settings;
	menu->type = GameMenuTypeVertSelect;

	strcpy( menu->title, "GAME SETTINGS" );
	menu->titleColor = TERM_YELLOW;

	strcpy( menu->rows[0], "Length of the code to resolve :   3   4   5   6" );
    strcpy( menu->rows[1], "Number of colors              :   4   5   6" );
    strcpy( menu->rows[2], "Allow duplicates colors       :   [yes]   no" );
    strcpy( menu->rows[3], "" /*Nothing to display here*/ );
    strcpy( menu->rows[4], "<-- Main menu" );
	strcpy( menu->rowSeparator, "\n" );
	menu->nbRows = 5;
	menu->currSelectedRow = 0;

	menu->lineColors[GameMenuStatus_DEFAULT] = TERM_WHITE;
	menu->lineColors[GameMenuStatus_FOCUS] = TERM_BOLD_GREEN;

	menu->hasPrefix = true;
	strcpy( menu->prefixes[GameMenuStatus_DEFAULT], "  " );
	strcpy( menu->prefixes[GameMenuStatus_FOCUS], "> " );

	menu->hasSuffix = false;
}


bool game_menu_init( struct GameMenuList *const menus )
{
	init_main_menu( &menus->menus[GameMenuId_MAIN] );
	init_menu_settings( &menus->menus[GameMenuId_Settings] );
}


void game_menu_shutdown( struct GameMenuList *const menus )
{
	// Nothing to do.
}


void game_menu_clear( struct GameMenu *const menu )
{
	bool const hasTitle = ( menu->title[0] != '\0' );
	usize const nbLines = menu->nbRows + (usize)( hasTitle );
	printf( "\033[%uA\r\033[J", nbLines );
}


void game_menu_print( struct GameMenu *const menu )
{
	if ( menu->title[0] != '\0' )
	{
		term_print( menu->titleColor, "%s\n", menu->title );
	}

	for ( usize i = 0; i < menu->nbRows; ++i )
	{
		enum GameMenuRowStatus const status = ( i == menu->currSelectedRow )
			? GameMenuStatus_FOCUS
			: GameMenuStatus_DEFAULT;

		char const *const prefix = menu->hasPrefix ? menu->prefixes[status] : "" ;
		char const *const suffix = menu->hasSuffix ? menu->suffixes[status] : "" ;

		// TODO Allow specifics colors for the prefix / separator ?
		// TODO We may also want a suffix ?
		printf( "%s%s%s%s%s%s",
			S_COLOR_STR[menu->lineColors[status]],
			prefix,
			menu->rows[i],
			suffix,
			S_COLOR_STR_RESET,
			menu->rowSeparator
		);
	}
}


// Need to merge together in one function, with more wrapper 
static bool main_menu_loop( struct GameMenu *const menu, INPUT_RECORD *input, enum GameMenuId *returnMenu )
{
	if ( input->EventType != KEY_EVENT ) return true;
	if ( !input->Event.KeyEvent.bKeyDown ) return true;

	if ( input->Event.KeyEvent.wVirtualKeyCode == KeyInput_ARROW_UP && menu->currSelectedRow > 0 )
	{
		menu->currSelectedRow -= 1;
		//game_menu_clear( menu );
	}
	else if ( input->Event.KeyEvent.wVirtualKeyCode == KeyInput_ARROW_DOWN && menu->currSelectedRow < menu->nbRows - 1 )
	{
		menu->currSelectedRow += 1;
		//game_menu_clear( menu );
	}
	else if ( input->Event.KeyEvent.wVirtualKeyCode == KeyInput_ENTER )
	{
		switch ( menu->currSelectedRow )
		{
			case 0: break;
			case 1: break;
			case 2: *returnMenu = GameMenuId_Settings; break;//game_menu_clear( menu ); break;
			case 3: *returnMenu = GameMenuId_Quit; break;
			default: break;
		}
		menu->currSelectedRow = 0;
		return false;
	}

	return true;
}


static bool game_settings_loop( struct GameMenu *const menu, INPUT_RECORD *input, enum GameMenuId *returnMenu )
{
	if ( input->EventType != KEY_EVENT ) return true;
	if ( !input->Event.KeyEvent.bKeyDown ) return true;

	if ( input->Event.KeyEvent.wVirtualKeyCode == KeyInput_ARROW_UP && menu->currSelectedRow > 0 )
	{
		// Just for the empty row. To Clean !
		menu->currSelectedRow -= menu->currSelectedRow == 4 ? 2 : 1;
		//game_menu_clear( menu );
	}
	else if ( input->Event.KeyEvent.wVirtualKeyCode == KeyInput_ARROW_DOWN && menu->currSelectedRow < menu->nbRows - 1 )
	{
		// Just for the empty row. to clean !
		menu->currSelectedRow += menu->currSelectedRow == 2 ? 2 : 1;
		//game_menu_clear( menu );
	}
	else if ( input->Event.KeyEvent.wVirtualKeyCode == KeyInput_ENTER )
	{
		if ( menu->currSelectedRow == 4 )
		{
			*returnMenu = GameMenuId_MAIN;
			//game_menu_clear( menu );
			menu->currSelectedRow = 0;
		}
		return false;
	}

	return true;
}


enum GameMenuId game_menu_enter( struct GameMenu *const menu )
{
	game_menu_print( menu );

	u8 oldSelect = menu->currSelectedRow;
	DWORD cNumRead;
	INPUT_RECORD irInBuf;
	HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
	bool stayInMenu = true;
	enum GameMenuId returnMenu = GameMenuId_Quit;
	while ( stayInMenu && ReadConsoleInput( hStdin, &irInBuf, 1, &cNumRead ) )
	{
		if ( menu->currSelectedRow != oldSelect )
		{
			game_menu_clear( menu );
			game_menu_print( menu );
			oldSelect = menu->currSelectedRow;
		}

		if ( menu->id == GameMenuId_MAIN ) stayInMenu = main_menu_loop( menu, &irInBuf, &returnMenu );
		if ( menu->id == GameMenuId_Settings ) stayInMenu = game_settings_loop( menu, &irInBuf, &returnMenu );
	}

	if ( returnMenu != GameMenuId_Quit )
	{
		game_menu_clear( menu );
	}
	return returnMenu;
}


void game_menu_loop( struct GameMenuList *const menus )
{
	while ( menus->current != GameMenuId_Quit )
	{
		menus->current = game_menu_enter( &menus->menus[menus->current] );
	}
}