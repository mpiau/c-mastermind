#pragma once

#include "rect.h"
#include "keyboard_inputs.h"

struct Button
{
    bool enabled;
    utf16 const *name;
    struct Rect rect;
    enum KeyInput bindedKey;
};

struct Button button_make( utf16 const *name, screenpos ul, vec2u16 size, enum KeyInput bindedKey, bool enabled );

void button_write( struct Button const *button, bool isHovered );

bool button_is_hovered_by( struct Button const *button, screenpos pos );

bool button_is_enabled( struct Button const *button );
void button_enable( struct Button *button );
void button_disable( struct Button *button );
