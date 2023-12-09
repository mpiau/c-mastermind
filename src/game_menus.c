#include "game_menus.h"
#include "keyboard_inputs.h"
#include "menu_stack.h"
#include "mastermind.h"

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <fcntl.h>


enum KeyInput term_next_user_input( void )
{
	DWORD cNumRead;
	INPUT_RECORD irInBuf;
	HANDLE const hStdin = GetStdHandle( STD_INPUT_HANDLE );
	while ( true )
	{
		bool const success = ReadConsoleInput( hStdin, &irInBuf, 1, &cNumRead );
		if ( !success )
		{
			fprintf( stderr, "[ERROR]: ReadConsoleInput failure : Code %u\n", GetLastError() );
			continue;
		}

		// We are only checking key events for this game, doesn't need to handle more cases.
		if ( irInBuf.EventType != KEY_EVENT ) continue;
		if ( !irInBuf.Event.KeyEvent.bKeyDown ) continue;

		enum KeyInput input;
		if ( !key_input_from_u32( irInBuf.Event.KeyEvent.wVirtualKeyCode, &input ) )
		{
			continue;
		}

		return input;
	}
}


static void init_main_menu( struct GameMenu *const menu )
{
	menu->id = GameMenuId_MAIN;
	menu->type = GameMenuTypeVertSelect;

	strcpy( menu->title, "MAIN MENU" );
	menu->titleColor = TERM_YELLOW;

	static_assert( ARR_COUNT( menu->rows ) >= 3 );
	strcpy( menu->rows[0], "New game" );
    strcpy( menu->rows[1], "Game Settings" );
    strcpy( menu->rows[2], "Quit" );
	strcpy( menu->rowSeparator, "\n" );
	menu->nbRows = 3;
	menu->currSelectedRow = 0;

	static_assert( ARR_COUNT( menu->lineColors ) >= 2 );
	menu->lineColors[GameMenuStatus_DEFAULT] = TERM_DEFAULT;
	menu->lineColors[GameMenuStatus_FOCUS] = TERM_BOLD_WHITE;

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
    strcpy( menu->rows[3], "Maximum mumber of turns       :   10 [-----------38-----] 50" );
	strcpy( menu->rows[4], "Number of players             :   1   2");
	strcpy( menu->rowSeparator, "\n" );
	menu->nbRows = 5;
	menu->currSelectedRow = 0;

	menu->lineColors[GameMenuStatus_DEFAULT] = TERM_DEFAULT;
	menu->lineColors[GameMenuStatus_FOCUS] = TERM_BOLD_WHITE;

	menu->hasPrefix = true;
	strcpy( menu->prefixes[GameMenuStatus_DEFAULT], "  " );
	strcpy( menu->prefixes[GameMenuStatus_FOCUS], "> " );

	menu->hasSuffix = false;
}


void termbuf_display( struct TermBuffer *const termBuf )
{
	printf( "%s", termBuf->buf );

	termBuf->curpos = snprintf( termBuf->buf, ARR_COUNT( termBuf->buf ), "\033[%uA\r\033[J", termBuf->nbLinesToClean );
	termBuf->nbLinesToClean = 0;
}


void termbuf_appendline( struct TermBuffer *const termBuf, char *const format, ... )
{
	va_list args;
	va_start ( args, format );
	termBuf->curpos += vsnprintf( termBuf->buf + termBuf->curpos, ARR_COUNT( termBuf->buf ) - termBuf->curpos, format, args );
	termBuf->curpos += snprintf( termBuf->buf + termBuf->curpos, ARR_COUNT( termBuf->buf ) - termBuf->curpos, "\n" );
	va_end ( args );

	termBuf->nbLinesToClean += 1;
}


void termbuf_init( struct TermBuffer *const termBuf )
{
	termBuf->curpos = 0;
	termBuf->nbLinesToClean = 0;
	termBuf->buf[0] = '\0';
}


bool game_menu_init( struct GameMenuList *const menus )
{
	termbuf_init( &menus->termBuf );
	for ( usize i = 0; i < ARR_COUNT( menus->menus ); ++i )
	{
		menus->menus[i] = (struct GameMenu) {};
	}
	init_main_menu( &menus->menus[GameMenuId_MAIN] );
	init_menu_settings( &menus->menus[GameMenuId_Settings] );
}


void game_menu_shutdown( struct GameMenuList *const menus )
{
	// Nothing to do.
}


void game_menu_print( struct GameMenu *const menu, struct TermBuffer *const termBuf )
{
	if ( menu->title[0] != '\0' )
	{
		termbuf_appendline( termBuf, "%s%s%s", S_COLOR_STR[TERM_YELLOW], menu->title, S_COLOR_STR_RESET );
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
		termbuf_appendline( termBuf, "%s%s%s%s%s",//%s",
			S_COLOR_STR[menu->lineColors[status]],
			prefix,
			menu->rows[i],
			suffix,
			S_COLOR_STR_RESET/*,
			menu->rowSeparator*/
		);
	}
}


static bool mastermind_singleplayer( struct GameMenu *const menu, enum GameMenuId *returnMenu, struct TermBuffer *const termBuf )
{
	struct Mastermind game;
	bool resetSettings = true;

	for ( ;; )
	{
		if ( !mastermind_init( &game, resetSettings ) )
		{
			printf( "Failed to initialize the game. Stopping.." );
			exit( 1 );
		}

		mastermind_game_start( &game, menu, termBuf );
		mastermind_destroy( &game );

		// Do you want to play again ?
		// Y -> loop again + Reset Settings ?
		// N -> break loop
		*returnMenu = GameMenuId_MAIN;
		break;
	}

	return true;
}


// Need to merge together in one function, with more wrapper 
static bool main_menu_loop( struct GameMenu *const menu, enum GameMenuId *returnMenu )
{
	enum KeyInput const keyInput = term_next_user_input();

	if ( keyInput == KeyInput_ARROW_UP && menu->currSelectedRow > 0 )
	{
		menu->currSelectedRow -= 1;
	}
	else if ( keyInput == KeyInput_ARROW_DOWN && menu->currSelectedRow < menu->nbRows - 1 )
	{
		menu->currSelectedRow += 1;
	}
	else if ( keyInput == KeyInput_ENTER )
	{
		switch ( menu->currSelectedRow )
		{
			case 0: *returnMenu = GameMenuId_Game; break;
			case 1: *returnMenu = GameMenuId_Settings; break;
			case 2: *returnMenu = GameMenuId_Quit; break;
			default: break;
		}
		menu->currSelectedRow = 0;
		return false;
	}
	else if ( keyInput == KeyInput_ESCAPE )
	{
		*returnMenu = GameMenuId_Quit;
		menu->currSelectedRow = 0;
		return false;
	}

	return true;
}


static bool game_settings_loop( struct GameMenu *const menu, enum GameMenuId *returnMenu )
{
	enum KeyInput const keyInput = term_next_user_input();

	if ( keyInput == KeyInput_ARROW_UP && menu->currSelectedRow > 0 )
	{
		menu->currSelectedRow -= 1;
	}
	else if ( keyInput == KeyInput_ARROW_DOWN && menu->currSelectedRow < menu->nbRows - 1 )
	{
		menu->currSelectedRow += 1;
	}
	else if ( keyInput == KeyInput_ENTER )
	{
		if ( menu->currSelectedRow == 4 )
		{
			*returnMenu = GameMenuId_MAIN;
			menu->currSelectedRow = 0;
		}
		return false;
	}
	else if ( keyInput == KeyInput_ESCAPE )
	{
		*returnMenu = GameMenuId_MAIN;
		menu->currSelectedRow = 0;
		return false;
	}

	return true;
}


enum GameMenuId game_menu_enter( struct GameMenu *const menu, struct TermBuffer *const termBuf )
{
	game_menu_print( menu, termBuf );
	termbuf_display( termBuf );

	u8 oldSelect = menu->currSelectedRow;
	enum GameMenuId returnMenu = GameMenuId_Quit;
	bool stayInMenu = true;
	while ( stayInMenu )
	{
		if ( menu->currSelectedRow != oldSelect )
		{
			game_menu_print( menu, termBuf );
			termbuf_display( termBuf );
			oldSelect = menu->currSelectedRow;
		}

		if ( menu->id == GameMenuId_MAIN ) stayInMenu = main_menu_loop( menu, &returnMenu );
		if ( menu->id == GameMenuId_Settings ) stayInMenu = game_settings_loop( menu, &returnMenu );
	}

	return returnMenu;
}


void game_menu_loop( struct GameMenuList *const menus )
{
	while ( menus->current != GameMenuId_Quit )
	{
		if ( menus->current == GameMenuId_Game )
		{
			enum GameMenuId returnMenu = GameMenuId_Quit;
			mastermind_singleplayer( &menus->menus[menus->current], &returnMenu, &menus->termBuf );
			continue;
		}

		menus->current = game_menu_enter( &menus->menus[menus->current], &menus->termBuf );
	}
}