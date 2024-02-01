#pragma once

#include "core/core.h"
#include "terminal/terminal_colors.h"


enum PegId
{
    PegId_BLACK,
    PegId_RED,
    PegId_GREEN,
    PegId_YELLOW,
    PegId_BLUE,
    PegId_MAGENTA,
    PegId_CYAN,
    PegId_WHITE,

    PegId_ColorsCount,

    PegId_EMPTY,

    PegId_Count
};


enum PinId
{
    PinId_CORRECT,
    PinId_PARTIAL,
    PinId_INCORRECT,

    PinId_Count
};


enum PieceType
{
    PieceType_PEG,
    PieceType_PIN
};


struct Peg
{
    enum PegId id;
    bool hidden;
};


struct Pin
{
    enum PinId id;
};


struct Piece
{
    enum PieceType type;
    union
    {
        struct Peg peg;
        struct Pin pin;
    };
};


void peg_write_1x1( screenpos pos, struct Peg peg );
void peg_write_4x2( screenpos pos, struct Peg peg );
void peg_write_6x3( screenpos pos, struct Peg peg, bool hovered );

void pin_write_1x1( screenpos pos, struct Pin pin );
void pin_write_4x2( screenpos pos, struct Pin pin );
void pin_write_6x3( screenpos pos, struct Pin pin, bool hovered );

void piece_write_1x1( screenpos pos, struct Piece piece );
void piece_write_4x2( screenpos pos, struct Piece piece );
void piece_write_6x3( screenpos pos, struct Piece piece, bool hovered );
