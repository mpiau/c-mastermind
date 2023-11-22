#include "keyboard_inputs.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


struct KeyInputData
{
	char const *name;
	u32 value;
};

// Based on https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes 

static const struct KeyInputData S_KEY_INPUT_DATA[] = 
{
	[KeyInput_ARROW_LEFT]  = (struct KeyInputData) { .value = VK_LEFT,  .name = "Left arrow" },
	[KeyInput_ARROW_RIGHT] = (struct KeyInputData) { .value = VK_RIGHT, .name = "Right arrow" },
	[KeyInput_ARROW_UP]    = (struct KeyInputData) { .value = VK_UP,    .name = "Up arrow" },
	[KeyInput_ARROW_DOWN]  = (struct KeyInputData) { .value = VK_DOWN,  .name = "Down arrow" },

	[KeyInput_0] = (struct KeyInputData) { .value = 0x30, .name = "0" },
	[KeyInput_1] = (struct KeyInputData) { .value = 0x31, .name = "1" },
	[KeyInput_2] = (struct KeyInputData) { .value = 0x32, .name = "2" },
	[KeyInput_3] = (struct KeyInputData) { .value = 0x33, .name = "3" },
	[KeyInput_4] = (struct KeyInputData) { .value = 0x34, .name = "4" },
	[KeyInput_5] = (struct KeyInputData) { .value = 0x35, .name = "5" },
	[KeyInput_6] = (struct KeyInputData) { .value = 0x36, .name = "6" },
	[KeyInput_7] = (struct KeyInputData) { .value = 0x37, .name = "7" },
	[KeyInput_8] = (struct KeyInputData) { .value = 0x38, .name = "8" },
	[KeyInput_9] = (struct KeyInputData) { .value = 0x39, .name = "9" },

	[KeyInput_NUMPAD_0] = (struct KeyInputData) { .value = 0x60, .name = "Numpad 0" },
	[KeyInput_NUMPAD_1] = (struct KeyInputData) { .value = 0x61, .name = "Numpad 1" },
	[KeyInput_NUMPAD_2] = (struct KeyInputData) { .value = 0x62, .name = "Numpad 2" },
	[KeyInput_NUMPAD_3] = (struct KeyInputData) { .value = 0x63, .name = "Numpad 3" },
	[KeyInput_NUMPAD_4] = (struct KeyInputData) { .value = 0x64, .name = "Numpad 4" },
	[KeyInput_NUMPAD_5] = (struct KeyInputData) { .value = 0x65, .name = "Numpad 5" },
	[KeyInput_NUMPAD_6] = (struct KeyInputData) { .value = 0x66, .name = "Numpad 6" },
	[KeyInput_NUMPAD_7] = (struct KeyInputData) { .value = 0x67, .name = "Numpad 7" },
	[KeyInput_NUMPAD_8] = (struct KeyInputData) { .value = 0x68, .name = "Numpad 8" },
	[KeyInput_NUMPAD_9] = (struct KeyInputData) { .value = 0x69, .name = "Numpad 9" },

	[KeyInput_A] = (struct KeyInputData) { .value = 0x41, .name = "A" },
	[KeyInput_B] = (struct KeyInputData) { .value = 0x42, .name = "B" },
	[KeyInput_C] = (struct KeyInputData) { .value = 0x43, .name = "C" },
	[KeyInput_D] = (struct KeyInputData) { .value = 0x44, .name = "D" },
	[KeyInput_E] = (struct KeyInputData) { .value = 0x45, .name = "E" },
	[KeyInput_F] = (struct KeyInputData) { .value = 0x46, .name = "F" },
	[KeyInput_G] = (struct KeyInputData) { .value = 0x47, .name = "G" },
	[KeyInput_H] = (struct KeyInputData) { .value = 0x48, .name = "H" },
	[KeyInput_I] = (struct KeyInputData) { .value = 0x49, .name = "I" },
	[KeyInput_J] = (struct KeyInputData) { .value = 0x4A, .name = "J" },
	[KeyInput_K] = (struct KeyInputData) { .value = 0x4B, .name = "K" },
	[KeyInput_L] = (struct KeyInputData) { .value = 0x4C, .name = "L" },
	[KeyInput_M] = (struct KeyInputData) { .value = 0x4D, .name = "M" },
	[KeyInput_N] = (struct KeyInputData) { .value = 0x4E, .name = "N" },
	[KeyInput_O] = (struct KeyInputData) { .value = 0x4F, .name = "O" },
	[KeyInput_P] = (struct KeyInputData) { .value = 0x50, .name = "P" },
	[KeyInput_Q] = (struct KeyInputData) { .value = 0x51, .name = "Q" },
	[KeyInput_R] = (struct KeyInputData) { .value = 0x52, .name = "R" },
	[KeyInput_S] = (struct KeyInputData) { .value = 0x53, .name = "S" },
	[KeyInput_T] = (struct KeyInputData) { .value = 0x54, .name = "T" },
	[KeyInput_U] = (struct KeyInputData) { .value = 0x55, .name = "U" },
	[KeyInput_V] = (struct KeyInputData) { .value = 0x56, .name = "V" },
	[KeyInput_W] = (struct KeyInputData) { .value = 0x57, .name = "W" },
	[KeyInput_X] = (struct KeyInputData) { .value = 0x58, .name = "X" },
	[KeyInput_Y] = (struct KeyInputData) { .value = 0x59, .name = "Y" },
	[KeyInput_Z] = (struct KeyInputData) { .value = 0x5A, .name = "Z" },

	[KeyInput_BACKSPACE] = (struct KeyInputData) { .value = VK_BACK,    .name = "Backspace" },
	[KeyInput_TAB]       = (struct KeyInputData) { .value = VK_TAB,     .name = "Tab" },
	[KeyInput_CLEAR]     = (struct KeyInputData) { .value = VK_CLEAR,   .name = "Clear" },
	[KeyInput_ENTER]     = (struct KeyInputData) { .value = VK_RETURN,  .name = "Enter" },
	[KeyInput_SHIFT]     = (struct KeyInputData) { .value = VK_SHIFT,   .name = "Shift" },
	[KeyInput_CONTROL]   = (struct KeyInputData) { .value = VK_CONTROL, .name = "Control" },
	[KeyInput_ALT]       = (struct KeyInputData) { .value = VK_MENU,    .name = "Alt" },
	[KeyInput_PAUSE]     = (struct KeyInputData) { .value = VK_PAUSE,   .name = "Pause" },
	[KeyInput_CAPSLOCK]  = (struct KeyInputData) { .value = VK_CAPITAL, .name = "Caps lock" },
	[KeyInput_ESCAPE]    = (struct KeyInputData) { .value = VK_ESCAPE,  .name = "Espace" },
	[KeyInput_SPACE]     = (struct KeyInputData) { .value = VK_SPACE,   .name = "Space" },
	[KeyInput_PAGE_UP]   = (struct KeyInputData) { .value = VK_PRIOR,   .name = "Page Up" },
	[KeyInput_PAGE_DOWN] = (struct KeyInputData) { .value = VK_NEXT,    .name = "Page Down" },
	[KeyInput_END]       = (struct KeyInputData) { .value = VK_END,     .name = "End" },
	[KeyInput_HOME]      = (struct KeyInputData) { .value = VK_HOME,    .name = "Home" },
	[KeyInput_INSERT]    = (struct KeyInputData) { .value = VK_INSERT,  .name = "Insert" },
	[KeyInput_DELETE]    = (struct KeyInputData) { .value = VK_DELETE,  .name = "Delete" },

	[KeyInput_MOUSE_BTN_LEFT]   = (struct KeyInputData) { .value = VK_LBUTTON, .name = "Left mouse button" },
	[KeyInput_MOUSE_BTN_RIGHT]  = (struct KeyInputData) { .value = VK_RBUTTON, .name = "Right mouse button" },
	[KeyInput_MOUSE_BTN_MIDDLE] = (struct KeyInputData) { .value = VK_MBUTTON, .name = "Middle mouse button" },
};

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
