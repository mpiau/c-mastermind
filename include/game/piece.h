#pragma once

#include "core/core.h"
#include "terminal/terminal_colors.h"

enum Piece
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

enum PieceTurn
{
    PieceTurn_IGNORE  = 0b00000000,
    PieceTurn_PAST    = 0b00010000,
    PieceTurn_CURRENT = 0b00100000,
    PieceTurn_FUTURE  = 0b00110000,

    PieceTurn_MaskAll = 0b00110000
};

enum PieceFlag
{
    PieceFlag_EMPTY  = 0b01000000, // The piece emplacement is not filled by a peg/pin.
    PieceFlag_SECRET = 0b10000000, // The piece has been hidden from the player (solution, ...)

    PieceFlag_MaskAll = 0b11000000
};

typedef byte gamepiece;

void piece_write_1x1( screenpos pos, gamepiece piece );
void piece_write_4x2( screenpos pos, gamepiece piece );
void piece_write_6x3( screenpos pos, gamepiece piece );
