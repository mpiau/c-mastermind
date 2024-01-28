#include "keybindings.h"

enum KeyInput keybinding_get_binded_key( enum KeyBinding keybinding )
{
    switch( keybinding )
    {
        case KeyBinding_QUIT:               return KeyInput_ESCAPE;
        case KeyBinding_OPEN_SETTINGS_MENU: return KeyInput_S;
        case KeyBinding_OPEN_GAME_RULES:    return KeyInput_G;
        case Keybinding_NEW_GAME:           return KeyInput_SPACE;
        case Keybinding_ABANDON_GAME:       return KeyInput_Q;
        case Keybinding_CONFIRM_TURN:       return KeyInput_ENTER;
        case Keybinding_RESET_TURN:         return KeyInput_R;

        case KeyBinding_PEG_BLACK:          return KeyInput_0;
        case KeyBinding_PEG_RED:            return KeyInput_1;
        case KeyBinding_PEG_GREEN:          return KeyInput_2;
        case KeyBinding_PEG_YELLOW:         return KeyInput_3;
        case KeyBinding_PEG_CYAN:           return KeyInput_4;
        case KeyBinding_PEG_MAGENTA:        return KeyInput_5;
        case KeyBinding_PEG_BLUE:           return KeyInput_6;
        case KeyBinding_PEG_WHITE:          return KeyInput_7;

        case KeyBinding_CLEAR_PEG:          return KeyInput_BACKSPACE;

        case KeyBinding_PREVIOUS:           return KeyInput_ARROW_LEFT;
        case KeyBinding_NEXT:               return KeyInput_ARROW_RIGHT;

        default:                            return KeyInput_INVALID;
    }
}
