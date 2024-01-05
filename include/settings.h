#pragma once

#include "core_types.h"

#include "mastermind.h"

bool settings_global_init( void );


u8 settings_get_number_turns( void );
u8 settings_get_pegs_per_turn( void );
enum GameExperience settings_get_game_experience( void );
bool settings_is_color_blind_mode_enabled( void );