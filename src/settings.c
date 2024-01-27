#include "settings.h"

struct Settings
{
    // General Settings
    usize cappedFramerate;

    // Relative to Mastermind
    usize gameNbTurns;
    usize gameNbPiecesPerTurn;
    enum GameExperience gameExperience;
    bool colorBlindMode;
    bool duplicateAllowed;
};

struct Settings s_settings = {};


bool settings_init( void )
{
    s_settings.cappedFramerate = 90; // -> Instead of having a getter, it will directly set it by calling the FPSCounter.

    // Changing it times to times in order to spot potential bugs in the display.
    bool result = true;

    result &= settings_set_nb_turns( Mastermind_MAX_TURNS );
    result &= settings_set_nb_pieces_per_turn( 4 );
    result &= settings_set_game_experience( GameExperience_NORMAL );
    result &= settings_set_color_blind_mode( false );
    result &= settings_set_duplicate_allowed( false );

    assert( result );
    return result;
}


bool settings_set_nb_turns( usize const nbTurns )
{
    if ( nbTurns < Mastermind_MIN_TURNS || nbTurns > Mastermind_MAX_TURNS )
    {
        return false;
    }

    s_settings.gameNbTurns = nbTurns;
    return true;
}

bool settings_set_nb_pieces_per_turn( usize const nbPieces )
{
    if ( nbPieces < Mastermind_MIN_PIECES_PER_TURN || nbPieces > Mastermind_MAX_PIECES_PER_TURN )
    {
        return false;
    }

    s_settings.gameNbPiecesPerTurn = nbPieces;
    return true;
}

bool settings_set_game_experience( enum GameExperience gameExperience )
{
    s_settings.gameExperience = gameExperience;
    return true;
}

bool settings_set_color_blind_mode( bool enabled )
{
    s_settings.colorBlindMode = enabled;
    return true;
}

bool settings_set_duplicate_allowed( bool allowed )
{
    s_settings.duplicateAllowed = allowed;
    return true;
}


// 

usize settings_get_nb_turns( void )
{
    return s_settings.gameNbTurns;
}

usize settings_get_nb_pieces_per_turn( void )
{
    return s_settings.gameNbPiecesPerTurn;
}

enum GameExperience settings_get_game_experience( void )
{
    return s_settings.gameExperience;
}


bool settings_is_color_blind_mode_enabled( void )
{
    return s_settings.colorBlindMode;
}


bool settings_is_duplicate_allowed( void )
{
    return s_settings.duplicateAllowed;
}
