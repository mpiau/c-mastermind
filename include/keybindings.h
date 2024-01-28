#pragma once

#include "core/core.h"
#include "keyboard_inputs.h"

enum KeyBinding
{
    KeyBinding_QUIT,
    KeyBinding_OPEN_SETTINGS_MENU,
    KeyBinding_OPEN_GAME_RULES,
    Keybinding_NEW_GAME,
    Keybinding_ABANDON_GAME,
    Keybinding_CONFIRM_TURN,
    Keybinding_RESET_TURN,

    KeyBinding_PEG_BLACK,
    KeyBinding_PEG_RED,
    KeyBinding_PEG_GREEN,
    KeyBinding_PEG_YELLOW,
    KeyBinding_PEG_CYAN,
    KeyBinding_PEG_MAGENTA,
    KeyBinding_PEG_BLUE,
    KeyBinding_PEG_WHITE,

    KeyBinding_CLEAR_PEG,

    KeyBinding_PREVIOUS,
    KeyBinding_NEXT,

    KeyBinding_Count
};

enum KeyInput keybinding_get_binded_key( enum KeyBinding keybinding );
