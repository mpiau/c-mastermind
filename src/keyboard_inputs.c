#include "keyboard_inputs.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


struct KeyInputData
{
    char const *name;
    u32 value;
};

// Based on https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes 

#define KEY_INPUT_DEF( _enum, _value, _name )								\
    [_enum] = (struct KeyInputData) { .value = _value, .name = _name }

static const struct KeyInputData S_KEY_INPUT_DATA[] = 
{
    KEY_INPUT_DEF( KeyInput_ARROW_LEFT , VK_LEFT , "Left"  ),
    KEY_INPUT_DEF( KeyInput_ARROW_RIGHT, VK_RIGHT, "Right" ),
    KEY_INPUT_DEF( KeyInput_ARROW_UP   , VK_UP   , "Up"    ),
    KEY_INPUT_DEF( KeyInput_ARROW_DOWN , VK_DOWN , "Down"  ),

    KEY_INPUT_DEF( KeyInput_0, 0x30, "0" ),
    KEY_INPUT_DEF( KeyInput_1, 0x31, "1" ),
    KEY_INPUT_DEF( KeyInput_2, 0x32, "2" ),
    KEY_INPUT_DEF( KeyInput_3, 0x33, "3" ),
    KEY_INPUT_DEF( KeyInput_4, 0x34, "4" ),
    KEY_INPUT_DEF( KeyInput_5, 0x35, "5" ),
    KEY_INPUT_DEF( KeyInput_6, 0x36, "6" ),
    KEY_INPUT_DEF( KeyInput_7, 0x37, "7" ),
    KEY_INPUT_DEF( KeyInput_8, 0x38, "8" ),
    KEY_INPUT_DEF( KeyInput_9, 0x39, "9" ),

    KEY_INPUT_DEF( KeyInput_NUMPAD_0, 0x60, "Numpad 0" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_1, 0x61, "Numpad 1" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_2, 0x62, "Numpad 2" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_3, 0x63, "Numpad 3" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_4, 0x64, "Numpad 4" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_5, 0x65, "Numpad 5" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_6, 0x66, "Numpad 6" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_7, 0x67, "Numpad 7" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_8, 0x68, "Numpad 8" ),
    KEY_INPUT_DEF( KeyInput_NUMPAD_9, 0x69, "Numpad 9" ),

    KEY_INPUT_DEF( KeyInput_A, 0x41, "A" ),
    KEY_INPUT_DEF( KeyInput_B, 0x42, "B" ),
    KEY_INPUT_DEF( KeyInput_C, 0x43, "C" ),
    KEY_INPUT_DEF( KeyInput_D, 0x44, "D" ),
    KEY_INPUT_DEF( KeyInput_E, 0x45, "E" ),
    KEY_INPUT_DEF( KeyInput_F, 0x46, "F" ),
    KEY_INPUT_DEF( KeyInput_G, 0x47, "G" ),
    KEY_INPUT_DEF( KeyInput_H, 0x48, "H" ),
    KEY_INPUT_DEF( KeyInput_I, 0x49, "I" ),
    KEY_INPUT_DEF( KeyInput_J, 0x4A, "J" ),
    KEY_INPUT_DEF( KeyInput_K, 0x4B, "K" ),
    KEY_INPUT_DEF( KeyInput_L, 0x4C, "L" ),
    KEY_INPUT_DEF( KeyInput_M, 0x4D, "M" ),
    KEY_INPUT_DEF( KeyInput_N, 0x4E, "N" ),
    KEY_INPUT_DEF( KeyInput_O, 0x4F, "O" ),
    KEY_INPUT_DEF( KeyInput_P, 0x50, "P" ),
    KEY_INPUT_DEF( KeyInput_Q, 0x51, "Q" ),
    KEY_INPUT_DEF( KeyInput_R, 0x52, "R" ),
    KEY_INPUT_DEF( KeyInput_S, 0x53, "S" ),
    KEY_INPUT_DEF( KeyInput_T, 0x54, "T" ),
    KEY_INPUT_DEF( KeyInput_U, 0x55, "U" ),
    KEY_INPUT_DEF( KeyInput_V, 0x56, "V" ),
    KEY_INPUT_DEF( KeyInput_W, 0x57, "W" ),
    KEY_INPUT_DEF( KeyInput_X, 0x58, "X" ),
    KEY_INPUT_DEF( KeyInput_Y, 0x59, "Y" ),
    KEY_INPUT_DEF( KeyInput_Z, 0x5A, "Z" ),

    KEY_INPUT_DEF( KeyInput_BACKSPACE , VK_BACK,    "Backspace" ),
    KEY_INPUT_DEF( KeyInput_TAB       , VK_TAB,     "Tab"       ),
    KEY_INPUT_DEF( KeyInput_CLEAR     , VK_CLEAR,   "Clear"     ),
    KEY_INPUT_DEF( KeyInput_ENTER     , VK_RETURN,  "Enter"     ),
    KEY_INPUT_DEF( KeyInput_SHIFT     , VK_SHIFT,   "Shift"     ),
    KEY_INPUT_DEF( KeyInput_CONTROL   , VK_CONTROL, "Ctrl"      ),
    KEY_INPUT_DEF( KeyInput_ALT       , VK_MENU,    "Alt"       ),
    KEY_INPUT_DEF( KeyInput_PAUSE     , VK_PAUSE,   "Pause"     ),
    KEY_INPUT_DEF( KeyInput_CAPSLOCK  , VK_CAPITAL, "Caps lock" ),
    KEY_INPUT_DEF( KeyInput_ESCAPE    , VK_ESCAPE,  "Esc"       ),
    KEY_INPUT_DEF( KeyInput_SPACE     , VK_SPACE,   "Space"     ),
    KEY_INPUT_DEF( KeyInput_PAGE_UP   , VK_PRIOR,   "Page Up"   ),
    KEY_INPUT_DEF( KeyInput_PAGE_DOWN , VK_NEXT,    "Page Down" ),
    KEY_INPUT_DEF( KeyInput_END       , VK_END,     "End"       ),
    KEY_INPUT_DEF( KeyInput_HOME      , VK_HOME,    "Home"      ),
    KEY_INPUT_DEF( KeyInput_INSERT    , VK_INSERT,  "Insert"    ),
    KEY_INPUT_DEF( KeyInput_DELETE    , VK_DELETE,  "Delete"    ),

    KEY_INPUT_DEF( KeyInput_MOUSE_BTN_LEFT  , VK_LBUTTON, "Left click"   ),
    KEY_INPUT_DEF( KeyInput_MOUSE_BTN_RIGHT , VK_RBUTTON, "Right click"  ),
    KEY_INPUT_DEF( KeyInput_MOUSE_BTN_MIDDLE, VK_MBUTTON, "Middle click" ),
};
#undef KEY_INPUT_DEF

static_assert( ARR_COUNT( S_KEY_INPUT_DATA ) == KeyInput_EnumCount );


bool key_input_from_u32( u32 const rawInput, enum KeyInput *const optKey )
{
    for ( usize i = 0; i < KeyInput_EnumCount; ++i )
    {
        if ( S_KEY_INPUT_DATA[i].value == rawInput )
        {
            if ( optKey ) *optKey = (enum KeyInput)( i );
            return true;
        }
    }

    return false;
}


char const *key_input_get_name( enum KeyInput const key )
{
    assert( key < KeyInput_EnumCount );
    return S_KEY_INPUT_DATA[key].name;
}


u32 key_input_get_raw_value( enum KeyInput const key )
{
    assert( key < KeyInput_EnumCount );
    return S_KEY_INPUT_DATA[key].value;
}


bool key_input_is_arrow( enum KeyInput const key )
{
    return key >= KeyInput_ArrowBegin && key <= KeyInput_ArrowEnd;
}


bool key_input_is_numpad( enum KeyInput const key )
{
    return key >= KeyInput_NumpadBegin && key <= KeyInput_NumpadEnd;
}


bool key_input_is_number( enum KeyInput const key )
{
    return key >= KeyInput_NumberBegin && key <= KeyInput_NumberEnd;
}


bool key_input_is_num( enum KeyInput const key )
{
    return key_input_is_number( key ) || key_input_is_numpad( key );
}


bool key_input_is_letter( enum KeyInput const key )
{
    return key >= KeyInput_LetterBegin && key <= KeyInput_LetterEnd;
}


bool key_input_is_special( enum KeyInput const key )
{
    return key >= KeyInput_SpecialBegin && key <= KeyInput_SpecialEnd;
}


bool key_input_is_mouse( enum KeyInput const key )
{
    return key >= KeyInput_MouseBegin && key <= KeyInput_MouseEnd;
}


char key_input_into_letter( enum KeyInput const key )
{
    assert( key_input_is_letter( key ) );

    return ( key - KeyInput_LetterBegin ) + 'A';
}


char key_input_into_num( enum KeyInput key )
{
    assert( key_input_is_num( key ) );

    u8 const base = key_input_is_number( key ) ? KeyInput_NumberBegin : KeyInput_NumpadBegin;
    return ( key - base ) + '0';
}
