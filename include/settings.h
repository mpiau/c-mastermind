#pragma once

#include "core_types.h"

#include "mastermind.h"

bool settings_init( void );

/* All these setters will be internal, and settings will check request instead */
bool settings_set_nb_turns( usize nbTurns );
bool settings_set_nb_pieces_per_turn( usize nbPieces );
bool settings_set_game_experience( enum GameExperience gameExperience );
bool settings_set_color_blind_mode( bool enabled );
bool settings_set_duplicate_allowed( bool allowed );

usize settings_get_nb_turns( void );
usize settings_get_nb_pieces_per_turn( void );
enum GameExperience settings_get_game_experience( void );
bool settings_is_color_blind_mode_enabled( void );
bool settings_is_duplicate_allowed( void );
