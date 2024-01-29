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

/*enum Piece
{
    Piece_PEG_BLACK   = 0b00000000,
    Piece_PEG_RED     = 0b00000001,
    Piece_PEG_GREEN   = 0b00000010,
    Piece_PEG_YELLOW  = 0b00000011,
    Piece_PEG_BLUE    = 0b00000100,
    Piece_PEG_MAGENTA = 0b00000101,
    Piece_PEG_CYAN    = 0b00000110,
    Piece_PEG_WHITE   = 0b00000111,

    Piece_PEGCount,

    Piece_TypePeg = 0b00000000,
    Piece_TypePin = 0b00001000,

    Piece_MaskColor = 0b00000111,
    Piece_MaskType  = 0b00001000,
    Piece_MaskAll   = Piece_MaskType | Piece_MaskColor,

    Piece_PIN_CORRECT   = Piece_MaskType | Piece_PEG_RED,
    Piece_PIN_PARTIAL   = Piece_MaskType | Piece_PEG_WHITE,
    Piece_PIN_INCORRECT = Piece_MaskType | Piece_PEG_BLACK,

};

enum PieceFlag
{
    PieceFlag_EMPTY  = 0b01000000, // The piece emplacement is not filled by a peg/pin.
    PieceFlag_SECRET = 0b10000000, // The piece has been hidden from the player (solution, ...)

    PieceFlag_MaskAll = 0b11000000
};

typedef byte gamepiece;*/

void peg_write_1x1( screenpos pos, struct Peg peg );
void peg_write_4x2( screenpos pos, struct Peg peg );
void peg_write_6x3( screenpos pos, struct Peg peg, bool hovered );

void pin_write_1x1( screenpos pos, struct Pin pin );
void pin_write_4x2( screenpos pos, struct Pin pin );
void pin_write_6x3( screenpos pos, struct Pin pin, bool hovered );

void piece_write_1x1( screenpos pos, struct Piece piece );
void piece_write_4x2( screenpos pos, struct Piece piece );
void piece_write_6x3( screenpos pos, struct Piece piece, bool hovered );
