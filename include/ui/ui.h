#pragma once

#include "core/core.h"


enum UIScene
{
    UIScene_NONE      = 0,
    UIScene_MAIN_MENU = 1 << 0,
    UIScene_IN_GAME   = 1 << 1,

    UIScene_ALL = UIScene_MAIN_MENU | UIScene_IN_GAME
};


bool ui_init( void );
void ui_uninit( void );

void ui_frame( void );
bool ui_change_scene( enum UIScene scene );
