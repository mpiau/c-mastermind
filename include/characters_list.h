#pragma once

// List of all the utf16 characters used to display the game in the screen console
// Boxes unicode have been taken from https://www.w3schools.com/charsets/ref_utf_box.asp
// The rest have been taken from https://www.compart.com/en/unicode/

enum UTF16Characters
{
    UTF16C_DoubleDownLeft   = L'╗',
    UTF16C_DoubleDownRight  = L'╔',
    UTF16C_DoubleUpLeft     = L'╝',
    UTF16C_DoubleUpRight    = L'╚',

    UTF16C_DoubleHoriz  = L'═',
    UTF16C_DoubleVert   = L'║',
    UTF16C_LightVert    = L'│',

    UTF16C_DoubleHorizSingleDown = L'╤',
    UTF16C_DoubleHorizSingleUp   = L'╧',
    UTF16C_DoubleHorizAndDown    = L'╦',
    UTF16C_DoubleHorizAndUp      = L'╩',

    UTF16C_SmallDottedCircle = L'◌',
    UTF16C_SmallFilledCircle = L'●',
    UTF16C_BigFilledCircle   = L'⬤',

    UTF16C_QuestionMark = L'?'
};
