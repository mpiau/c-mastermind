#pragma once

#include "core_types.h"

enum
{
    ONE_SEC      = 1,
    ONE_MILLISEC = ONE_SEC      * 1000,
    ONE_MICROSEC = ONE_MILLISEC * 1000,
    ONE_NANOSEC  = ONE_MICROSEC * 1000
};

struct FPSCounter;

struct FPSCounter *fpscounter_init( void );
void fpscounter_uninit( struct FPSCounter *fpsCounter );

void fpscounter_frame_begin( struct FPSCounter *fpsCounter );
u64 fpscounter_frame_end( struct FPSCounter *fpsCounter );

u64 fpscounter_elapsed_time_ns( struct FPSCounter *fpsCounter );
