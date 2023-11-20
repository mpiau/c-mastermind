#pragma once

#include "core_types.h"

// Based on https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes 

enum KeyInput
{
	KeyInput_MOUSE_BTN_LEFT		= 0x01,
	KeyInput_MOUSE_BTN_RIGHT	= 0x02,
	KeyInput_MOUSE_BTN_MIDDLE	= 0x04,

	KeyInput_BACKSPACE			= 0x08,
	KeyInput_TAB				= 0x09,
	KeyInput_CLEAR				= 0x0C,
	KeyInput_ENTER				= 0x0D,
	KeyInput_SHIFT				= 0x10,
	KeyInput_CONTROL			= 0x11,
	KeyInput_ALT				= 0x12,
	KeyInput_PAUSE				= 0x13,
	KeyInput_CAPSLOCK			= 0x14,
	KeyInput_ESCAPE				= 0x1B,
	KeyInput_SPACE				= 0x20,
	KeyInput_PAGE_UP			= 0x21,
	KeyInput_PAGE_DOWN			= 0x22,
	KeyInput_END				= 0x23,
	KeyInput_HOME				= 0x24,

	KeyInput_ARROW_LEFT			= 0x25,
	KeyInput_ARROW_UP			= 0x26,
	KeyInput_ARROW_RIGHT		= 0x27,
	KeyInput_ARROW_DOWN			= 0x28,
	KeyInput_ArrowBegin			= KeyInput_ARROW_LEFT,
	KeyInput_ArrowEnd			= KeyInput_ARROW_DOWN,

	KeyInput_INSERT				= 0x2D,
	KeyInput_DELETE				= 0x2E,

	KeyInput_0					= 0x30,
	KeyInput_1					= 0x31,
	KeyInput_2					= 0x32,
	KeyInput_3					= 0x33,
	KeyInput_4					= 0x34,
	KeyInput_5					= 0x35,
	KeyInput_6					= 0x36,
	KeyInput_7					= 0x37,
	KeyInput_8					= 0x38,
	KeyInput_9					= 0x39,
	KeyInput_NumberBegin		= KeyInput_0,
	KeyInput_NumberEnd			= KeyInput_9,

	KeyInput_A					= 0x41,
	KeyInput_B					= 0x42,
	KeyInput_C					= 0x43,
	KeyInput_D					= 0x44,
	KeyInput_E					= 0x45,
	KeyInput_F					= 0x46,
	KeyInput_G					= 0x47,
	KeyInput_H					= 0x48,
	KeyInput_I					= 0x49,
	KeyInput_J					= 0x4A,
	KeyInput_K					= 0x4B,
	KeyInput_L					= 0x4C,
	KeyInput_M					= 0x4D,
	KeyInput_N					= 0x4E,
	KeyInput_O					= 0x4F,
	KeyInput_P					= 0x50,
	KeyInput_Q					= 0x51,
	KeyInput_R					= 0x52,
	KeyInput_S					= 0x53,
	KeyInput_T					= 0x54,
	KeyInput_U					= 0x55,
	KeyInput_V					= 0x56,
	KeyInput_W					= 0x57,
	KeyInput_X					= 0x58,
	KeyInput_Y					= 0x59,
	KeyInput_Z					= 0x5A,
	KeyInput_LetterBegin		= KeyInput_A,
	KeyInput_LetterEnd			= KeyInput_Z,

	KeyInput_NUMPAD_0			= 0x60,
	KeyInput_NUMPAD_1			= 0x61,
	KeyInput_NUMPAD_2			= 0x62,
	KeyInput_NUMPAD_3			= 0x63,
	KeyInput_NUMPAD_4			= 0x64,
	KeyInput_NUMPAD_5			= 0x65,
	KeyInput_NUMPAD_6			= 0x66,
	KeyInput_NUMPAD_7			= 0x67,
	KeyInput_NUMPAD_8			= 0x68,
	KeyInput_NUMPAD_9			= 0x69,
	KeyInput_NumpadBegin		= KeyInput_NUMPAD_0,
	KeyInput_NumpadEnd			= KeyInput_NUMPAD_9,

	KeyInput_EnumCount // Implementation only.
};


char const *key_try_get_name( enum KeyInput input );

inline bool key_is_letter( enum KeyInput const input )
{
	return input >= KeyInput_LetterBegin && input <= KeyInput_LetterEnd;
}

inline bool key_is_numpad( enum KeyInput const input )
{
	return input >= KeyInput_NumpadBegin && input <= KeyInput_NumpadEnd;
}


inline bool key_is_number( enum KeyInput const input )
{
	return input >= KeyInput_NumberBegin && input <= KeyInput_NumberEnd;
}


inline bool key_is_num( enum KeyInput const input )
{
	return key_is_number( input ) || key_is_numpad( input );
}


inline u8 key_num_to_array_idx( enum KeyInput const input )
{
	if      ( key_is_number( input ) ) return input - KeyInput_NumberBegin;
	else if ( key_is_numpad( input ) ) return input - KeyInput_NumpadEnd;

	assert( false );
	return 0;
}
