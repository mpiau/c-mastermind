#pragma once

#include "core/core.h"
#include "terminal/terminal_colors.h"

enum Piece
{
    Piece_PEG_BLACK   = 0b00000000,
    Piece_PEG_RED     = 0b00000001,
    Piece_PEG_GREEN   = 0b00000010,
    Piece_PEG_YELLOW  = 0b00000011,
    Piece_PEG_CYAN    = 0b00000100,
    Piece_PEG_MAGENTA = 0b00000101,
    Piece_PEG_BLUE    = 0b00000110,
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

/*
enum PegId
{
    PegId_BLACK,
    PegId_RED,
    PegId_GREEN,
    PegId_YELLOW,
    PegId_CYAN,
    PegId_MAGENTA,
    PegId_BLUE,
    PegId_WHITE,

    PegId_Count,
    PegId_Empty = PegId_Count // Doesn't count as a valid color id, but can be used to draw an empty peg space.
};


enum PegSize
{
    PegSize_WIDTH = 6,
    PegSize_HEIGHT = 3
};


struct Peg
{
    enum PegId id;
    bool hidden;
};

// The peg has a size of PegSize_WIDTH x PegSize_HEIGHT
void peg_draw( struct Peg const *peg, usize ulX, usize ulY );

// Minimalist version of the peg, drawn in a single character
void peg_draw_single_character( struct Peg const *peg, usize posX, usize posY, bool isFutureTurn );


// Pins part

enum PinId
{
    PinId_CORRECT,           // Correct color + correct position
    PinId_PARTIALLY_CORRECT, // Correct color + incorrect position
    PinId_INCORRECT,         // both incorrect

    PinId_Count,
};

struct Pin
{
    enum PinId id;
};

struct Style;
struct Style pin_get_style( enum PinId id );
*/
