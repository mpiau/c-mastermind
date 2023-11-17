#pragma once

#include "core_types.h"
#include "terminal.h"

// TODO Not the same enum convention, but I like that. Needs to set some consistency though.

enum GameMenuId
{
	GameMenuId_Main,
	GameMenuId_SettingsSP,
	GameMenuId_SettingsMP,
	GameMenuId_Pause,

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
};

enum GameMenuRowStatus
{
	GameMenuRowStatus_DEFAULT,
	GameMenuRowStatus_SELECTED,

	GameMenuRowStatus_COUNT
};


struct GameMenu
{
	enum GameMenuId   id;
	enum GameMenuType type;

	char rows[GameMenu_MAX_ROWS][GameMenu_MAX_ROW_SIZE];
	char rowSeparator[GameMenu_MAX_ROW_SIZE];
	u8 nbRows;
	u8 currSelectedRow;

	enum TermColor lineColors[GameMenuRowStatus_COUNT];

	bool hasPrefix;
	char prefixes[GameMenuRowStatus_COUNT][GameMenu_MAX_PREFIX_SIZE];
};


struct GameMenuList
{
	struct GameMenu menus[GameMenuId_COUNT];
	enum GameMenuId current;
};


bool game_menu_init( struct GameMenuList *const menus );
void game_menu_shutdown( struct GameMenuList *const menus );
void game_menu_set_curr( struct GameMenuList *const menus, enum GameMenuId id );
void game_menu_loop( struct GameMenuList *const menus );
