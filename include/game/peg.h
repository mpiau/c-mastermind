#pragma once

#include "core_types.h"
#include "core_unions.h"

enum PegId
{
    PegId_RED,
    PegId_GREEN,
    PegId_YELLOW,
    PegId_CYAN,
    PegId_MAGENTA,
    PegId_BLUE,
    PegId_WHITE,
    PegId_BLACK,

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
void peg_draw_single_character( struct Peg const *peg, usize posX, usize posY );
