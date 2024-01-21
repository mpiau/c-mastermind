#pragma once

#include "core/core.h"
#include "keyboard_inputs.h"

enum KeyBinding
{
    KeyBinding_QUIT,
    KeyBinding_OPEN_SETTINGS_MENU,
    KeyBinding_OPEN_GAME_RULES,
    Keybinding_NEW_GAME,

    KeyBinding_Count
};

enum KeyInput keybinding_get_binded_key( enum KeyBinding keybinding );
