#pragma once

#include "core_types.h"
#include "core_unions.h"

struct MastermindV2;

bool widget_board_summary_init( struct MastermindV2 const *mastermind );
void widget_board_summary_uninit( void );
