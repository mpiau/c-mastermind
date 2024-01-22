#pragma once

#include "core_types.h"

enum KeyInput
{
    KeyInput_ARROW_LEFT  ,
    KeyInput_ARROW_RIGHT ,
    KeyInput_ARROW_UP    ,
    KeyInput_ARROW_DOWN  ,
    KeyInput_ArrowBegin  = KeyInput_ARROW_LEFT,
    KeyInput_ArrowEnd    = KeyInput_ARROW_DOWN,

    KeyInput_0           ,
    KeyInput_1           ,
    KeyInput_2           ,
    KeyInput_3           ,
    KeyInput_4           ,
    KeyInput_5           ,
    KeyInput_6           ,
    KeyInput_7           ,
    KeyInput_8           ,
    KeyInput_9           ,
    KeyInput_NumberBegin = KeyInput_0,
    KeyInput_NumberEnd   = KeyInput_9,

    KeyInput_NUMPAD_0    ,
    KeyInput_NUMPAD_1    ,
    KeyInput_NUMPAD_2    ,
    KeyInput_NUMPAD_3    ,
    KeyInput_NUMPAD_4    ,
    KeyInput_NUMPAD_5    ,
    KeyInput_NUMPAD_6    ,
    KeyInput_NUMPAD_7    ,
    KeyInput_NUMPAD_8    ,
    KeyInput_NUMPAD_9    ,
    KeyInput_NumpadBegin = KeyInput_NUMPAD_0,
    KeyInput_NumpadEnd   = KeyInput_NUMPAD_9,

    KeyInput_A           ,
    KeyInput_B           ,
    KeyInput_C           ,
    KeyInput_D           ,
    KeyInput_E           ,
    KeyInput_F           ,
    KeyInput_G           ,
    KeyInput_H           ,
    KeyInput_I           ,
    KeyInput_J           ,
    KeyInput_K           ,
    KeyInput_L           ,
    KeyInput_M           ,
    KeyInput_N           ,
    KeyInput_O           ,
    KeyInput_P           ,
    KeyInput_Q           ,
    KeyInput_R           ,
    KeyInput_S           ,
    KeyInput_T           ,
    KeyInput_U           ,
    KeyInput_V           ,
    KeyInput_W           ,
    KeyInput_X           ,
    KeyInput_Y           ,
    KeyInput_Z           ,
    KeyInput_LetterBegin = KeyInput_A,
    KeyInput_LetterEnd   = KeyInput_Z,

    KeyInput_BACKSPACE    ,
    KeyInput_TAB          ,
    KeyInput_CLEAR        ,
    KeyInput_ENTER        ,
    KeyInput_SHIFT        ,
    KeyInput_CONTROL      ,
    KeyInput_ALT          ,
    KeyInput_PAUSE        ,
    KeyInput_CAPSLOCK     ,
    KeyInput_ESCAPE       ,
    KeyInput_SPACE        ,
    KeyInput_PAGE_UP      ,
    KeyInput_PAGE_DOWN    ,
    KeyInput_END          ,
    KeyInput_HOME         ,
    KeyInput_INSERT       ,
    KeyInput_DELETE       ,
    KeyInput_SpecialBegin = KeyInput_BACKSPACE,
    KeyInput_SpecialEnd   = KeyInput_DELETE,

    KeyInput_MOUSE_BTN_LEFT   ,
    KeyInput_MOUSE_BTN_RIGHT  ,
    KeyInput_MOUSE_BTN_MIDDLE ,
    KeyInput_MouseBegin       = KeyInput_MOUSE_BTN_LEFT,
    KeyInput_MouseEnd         = KeyInput_MOUSE_BTN_MIDDLE,

    KeyInput_EnumCount, // Implementation only.

    KeyInput_INVALID = KeyInput_EnumCount
};


// Returns true if the raw input is mapped.
// If optKey is not NULL, the associated key input will also be given.
bool key_input_from_u32( u32 rawInput, enum KeyInput *const optKey );

char const *key_input_get_name( enum KeyInput key );
u32 key_input_get_raw_value( enum KeyInput key );

bool key_input_is_arrow( enum KeyInput key );
bool key_input_is_numpad( enum KeyInput key );
bool key_input_is_number( enum KeyInput key );
bool key_input_is_num( enum KeyInput key );
bool key_input_is_letter( enum KeyInput key );
bool key_input_is_special( enum KeyInput key );
bool key_input_is_mouse( enum KeyInput key );

// Must satisfy key_input_is_numpad.
enum KeyInput key_input_from_numpad_to_number( enum KeyInput key );

// Must satisfy key_input_is_letter.
char key_input_into_letter( enum KeyInput key );

// Must satisfy key_input_is_num.
char key_input_into_num( enum KeyInput key );
