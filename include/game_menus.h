#pragma once

#include "core_types.h"
#include "terminal.h"
#include "keyboard_inputs.h"
// TODO Not the same enum convention, but I like that. Needs to set some consistency though.

enum GameMenuId
{
	GameMenuId_MAIN,
	GameMenuId_SetupMP,
	GameMenuId_Settings,
	GameMenuId_HowToPlay,
	GameMenuId_Pause,
	GameMenuId_Game,

	GameMenuId_COUNT,

	GameMenuId_Quit = GameMenuId_COUNT
};


enum GameMenuType
{
	GameMenuTypeVertSelect,
	GameMenuTypeHorizSelect,

	GameMenuType_COUNT,
};


enum // Constants
{
	GameMenu_MAX_ROWS = 5,
	GameMenu_MAX_ROW_SIZE = 64,
	GameMenu_MAX_PREFIX_SIZE = 8,
	GameMenu_MAX_SUFFIX_SIZE = GameMenu_MAX_PREFIX_SIZE,
	GameMenu_MAX_TITLE_SIZE = 64
};

enum GameMenuRowStatus
{
	GameMenuStatus_DEFAULT,
	GameMenuStatus_FOCUS,

	GameMenuRowStatus_COUNT
};


// TODO add id for each element.


struct GameMenu
{
	enum GameMenuId   id;
	enum GameMenuType type;

	char title[GameMenu_MAX_TITLE_SIZE];
	enum TermColor titleColor;

	char rows[GameMenu_MAX_ROWS][GameMenu_MAX_ROW_SIZE];
	char rowSeparator[GameMenu_MAX_ROW_SIZE];
	u8 nbRows;
	u8 currSelectedRow;

	enum TermColor lineColors[GameMenuRowStatus_COUNT];

	bool hasPrefix;
	char prefixes[GameMenuRowStatus_COUNT][GameMenu_MAX_PREFIX_SIZE];
	bool hasSuffix;
	char suffixes[GameMenuRowStatus_COUNT][GameMenu_MAX_SUFFIX_SIZE];
};

struct TermBuffer
{
	char  buf[1024];
	usize curpos;
	usize nbLinesToClean;
};


struct GameMenuList
{
	struct GameMenu menus[GameMenuId_COUNT];
	enum GameMenuId current;

	struct TermBuffer termBuf;
};


bool game_menu_init( struct GameMenuList *const menus );
void game_menu_shutdown( struct GameMenuList *const menus );
void game_menu_set_curr( struct GameMenuList *const menus, enum GameMenuId id );
void game_menu_loop( struct GameMenuList *const menus );

void termbuf_display( struct TermBuffer *const termBuf );
void termbuf_appendline( struct TermBuffer *const termBuf, char *const format, ... );
enum KeyInput term_next_user_input( void );