#pragma once

#include "core_types.h"


enum // Constants
{
	PEGS_CODE_LENGTH_MIN = 3,
	PEGS_CODE_LENGTH_MAX = 6,

	PEGS_COLORS_NUMBER_MIN = 5,
	PEGS_COLORS_NUMBER_MAX = 10,

	TURNS_NUMBER_MIN = 8,
	TURNS_NUMBER_MAX = 50
};

enum MastermindGameStatus
{
	GAME_STATUS_NOT_STARTED = 0,
	GAME_STATUS_ONGOING,
	GAME_STATUS_DEFEAT,
	GAME_STATUS_VICTORY
};


struct MastermindSettings
{
	u8 pegsCodeLength;
	u8 colorsNumber;
	u8 turnsNumber;
	bool allowDuplicatePegs;
};


struct Mastermind
{
	struct MastermindSettings settings;
	u8 codemaker[PEGS_CODE_LENGTH_MAX];
	u8 codebreaker[PEGS_CODE_LENGTH_MAX];
	u8 currentTurn;
	enum MastermindGameStatus status;
	// Idea: Dynamic allocation with the turn history ?
};


