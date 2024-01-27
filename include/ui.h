#pragma once

#include "rect.h"
#include "keyboard_inputs.h"
#include "terminal/terminal.h"
#include "keybindings.h"

typedef void ( *OnButtonTriggerCb )( bool clicked );

struct UIButton
{
    bool active;
    bool shown;
    struct Rect rect;
    utf16 const *text;
    enum KeyBinding keybinding;
    OnButtonTriggerCb onTriggerCb;
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


u64  uibutton_register( utf16 const *text, screenpos pos, vec2u16 size, enum KeyBinding keybinding, OnButtonTriggerCb onTriggerCb, bool active );
bool uibutton_check_interaction( u64 id, enum KeyInput input );
bool uibutton_is_active( u64 id );
void uibutton_activate( u64 id );
void uibutton_desactivate( u64 id );
bool uibutton_is_hovered_by( u64 id, screenpos pos );
bool uibutton_check_hovered( u64 id, screenpos pos );
struct Rect const *uibutton_get_box( u64 id );

void uibutton_show( u64 id );
void uibutton_hide( u64 id );
