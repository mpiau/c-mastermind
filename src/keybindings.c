#include "keybindings.h"

enum KeyInput keybinding_get_binded_key( enum KeyBinding keybinding )
{
    switch( keybinding )
    {
        case KeyBinding_QUIT:               return KeyInput_ESCAPE;
        case KeyBinding_OPEN_SETTINGS_MENU: return KeyInput_S;
        case KeyBinding_OPEN_GAME_RULES:    return KeyInput_G;
        case Keybinding_NEW_GAME:           return KeyInput_SPACE;

        default:                            return KeyInput_INVALID;
    }
}
