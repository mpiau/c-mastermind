#pragma once

#include "core_types.h"


enum TermColor
{
	TERM_BLACK = 0,
	TERM_RED,
	TERM_GREEN,
	TERM_YELLOW,
	TERM_BLUE,
	TERM_MAGENTA,
	TERM_CYAN,
	TERM_WHITE,

	TERM_BOLD_BLACK,
	TERM_BOLD_RED,
	TERM_BOLD_GREEN,
	TERM_BOLD_YELLOW,
	TERM_BOLD_BLUE,
	TERM_BOLD_MAGENTA,
	TERM_BOLD_CYAN,
	TERM_BOLD_WHITE,

	TERM_DEFAULT,

	TERM_COUNT,

	TERM_COLOR_NONE,
};

// Temp, but easier to work with for now.

static char const *const S_COLOR_STR[] =
{
	[TERM_BLACK]   = "\x1b[30m",
    [TERM_RED]     = "\x1b[31m",
    [TERM_GREEN]   = "\x1b[32m",
    [TERM_YELLOW]  = "\x1b[33m",
    [TERM_BLUE]    = "\x1b[34m",
    [TERM_MAGENTA] = "\x1b[35m",
    [TERM_CYAN]    = "\x1b[36m",
    [TERM_WHITE]   = "\x1b[37m",

    [TERM_BOLD_BLACK]   = "\x1b[1;30m",
    [TERM_BOLD_RED]     = "\x1b[1;31m",
    [TERM_BOLD_GREEN]   = "\x1b[1;32m",
    [TERM_BOLD_YELLOW]  = "\x1b[1;33m",
    [TERM_BOLD_BLUE]    = "\x1b[1;34m",
    [TERM_BOLD_MAGENTA] = "\x1b[1;35m",
    [TERM_BOLD_CYAN]    = "\x1b[1;36m",
    [TERM_BOLD_WHITE]   = "\x1b[1;37m",

    [TERM_DEFAULT] = ""
};
static_assert( ARR_COUNT( S_COLOR_STR ) == TERM_COUNT );
static char const *const S_COLOR_STR_RESET = "\x1b[0;0m";
