#include "settings.h"

struct Settings
{
    // General Settings
    u64 cappedFramerate;

    // Relative to Mastermind
    u8 gameNbTurns;
    u8 gameNbPegsPerTurn;
    enum GameExperience gameExperience;
    bool colorBlindMode;
};

struct Settings s_settings = {};


bool settings_global_init( void )
{
    s_settings.cappedFramerate = 60; // -> Instead of having a getter, it will directly set it by calling the FPSCounter.
    s_settings.gameExperience = GameExperience_NORMAL;

    // Changing it times to times in order to spot potential bugs in the display.
    s_settings.gameNbPegsPerTurn = 4;
    s_settings.gameNbTurns = 8;
    s_settings.colorBlindMode = false;
}


u8 settings_get_number_turns( void )
{
    return s_settings.gameNbTurns;
}


u8 settings_get_pegs_per_turn( void )
{
    return s_settings.gameNbPegsPerTurn;
}


enum GameExperience settings_get_game_experience( void )
{
    return s_settings.gameExperience;
}


bool settings_is_color_blind_mode_enabled( void )
{
    return s_settings.colorBlindMode;
}