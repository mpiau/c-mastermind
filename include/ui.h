#pragma once

#include "rect.h"
#include "keyboard_inputs.h"
#include "terminal/terminal.h"
#include "keybindings.h"


struct UIButton
{
    bool enabled;
    bool shown;
    struct Rect rect;
    utf16 const *text;
    enum KeyBinding keybinding;
};

struct UIPiece
{
    // ...
};

struct UISimpleText
{
    struct Rect rect;
    utf16 const *text;
};


u64 uibutton_try_register( utf16 const *text, screenpos pos, vec2u16 size, enum KeyBinding keybinding, bool enabled );
bool uibutton_clicked( u64 id );
bool uibutton_is_enabled( u64 id );
void uibutton_enable( u64 id );
void uibutton_disable( u64 id );
bool uibutton_is_hovered_by( u64 id, screenpos pos );
bool uibutton_check_hovered( u64 id, screenpos pos );
struct Rect const *uibutton_get_box( u64 id );

void uibutton_show( u64 id );
void uibutton_hide( u64 id );
