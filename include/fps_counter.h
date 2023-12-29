#pragma once

#include "core_types.h"
#include "time_units.h"

struct FPSCounter;

struct FPSCounter *fpscounter_init( void );
void fpscounter_uninit( struct FPSCounter *fpsCounter );
struct FPSCounter *fpscounter_get_instance( void );

u64 fpscounter_frame( struct FPSCounter *fpsCounter );

void fpscounter_frame_begin( struct FPSCounter *fpsCounter );
u64 fpscounter_frame_end( struct FPSCounter *fpsCounter );

nsec fpscounter_elapsed_time( struct FPSCounter *fpsCounter );
nsec fpscounter_average_time( struct FPSCounter *fpsCounter );
u64 fpscounter_average_framerate( struct FPSCounter *fpsCounter );
