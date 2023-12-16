#include "fps_counter.h"

#include <math.h>
#include <time.h>
#include <synchapi.h>
#include <windows.h>


// Sleep(0) or more - no
// SleepEx - no
// nanosleep - no
// Another one has a Mincore.lib dependency
// timeBeginPeriod / timeEndPeriod -> No, don't want to modify the whole system and consume more energy, so waitable timer is nice.

// Not defined in GCC - Windows toolchain
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

enum
{
    NB_FRAMES_HISTORY = 8
};

struct FPSHistory
{
    u64 lastFramesDurationNs[NB_FRAMES_HISTORY];
    u64 totalFramesDurationNs;
    u64 averageFrameDurationNs;
    u8  index;
};

struct FPSCounter
{
    HANDLE waitableTimer;
    LARGE_INTEGER minWaitTimePerFrameNs;

    u64 frameBeginNs;
    u64 frameEndNs;

    struct FPSHistory history;
};

static struct FPSCounter s_fpsCounter = {}; // Just to avoid dynamic alloc

static float const MS_TO_NANO = 1000000.0f;
static float const NANO_TO_MS = 0.000001f;


static u64 get_timestamp_nanoseconds()
{
    struct timespec time;
    clock_gettime( CLOCK_MONOTONIC, &time );
    return time.tv_sec * ONE_NANOSEC  + time.tv_nsec; // time.tv_sec * 1000000 + time.tv_nsec / 1000;
}

struct FPSCounter *fpscounter_init( void )
{
    struct FPSCounter *fpsCounter = &s_fpsCounter;
    fpsCounter->waitableTimer = CreateWaitableTimerExW( NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS );
    if ( fpsCounter->waitableTimer == NULL || fpsCounter->waitableTimer == INVALID_HANDLE_VALUE )
    {
        return NULL;
    }

    u64 const FPS_60_NANO = (u64)roundf( ( ONE_NANOSEC ) / 60.0f );
    // - 1 ms for accuracy, otherwise we will be at 59fps instead of 60fps
    // negative to be relative and not UTC.
    fpsCounter->minWaitTimePerFrameNs.QuadPart = (i64)( (u64)( FPS_60_NANO - 1000000 ) / (u64)100 ) * -1;
    return fpsCounter;
}


void fpscounter_uninit( struct FPSCounter *fpsCounter )
{
    if ( fpsCounter && fpsCounter->waitableTimer != INVALID_HANDLE_VALUE )
    {
        CloseHandle( fpsCounter->waitableTimer );
    }
}


void fpscounter_frame_begin( struct FPSCounter *fpsCounter )
{
    SetWaitableTimerEx( fpsCounter->waitableTimer, &fpsCounter->minWaitTimePerFrameNs, 0, NULL, NULL, NULL, 0 );
	fpsCounter->frameBeginNs = get_timestamp_nanoseconds();
}


u64 fpscounter_frame_end( struct FPSCounter *fpsCounter )
{
    WaitForSingleObject( fpsCounter->waitableTimer, INFINITE );

    u64 const FPS_60_NANO = (u64)roundf( ( ONE_NANOSEC ) / 60.0f );
	do
	{
		fpsCounter->frameEndNs = get_timestamp_nanoseconds();
	} while ( fpsCounter->frameEndNs - fpsCounter->frameBeginNs < FPS_60_NANO );

	u64 deltaNano = fpsCounter->frameEndNs - fpsCounter->frameBeginNs;
	u64 framerate = deltaNano;

    struct FPSHistory *history = &fpsCounter->history;
    history->totalFramesDurationNs -= history->lastFramesDurationNs[history->index];
    history->totalFramesDurationNs += framerate;
    history->lastFramesDurationNs[history->index] = framerate;
    history->index = ( history->index + 1 ) % NB_FRAMES_HISTORY;
    history->averageFrameDurationNs = history->totalFramesDurationNs / NB_FRAMES_HISTORY;

    return (u64)( roundf( 1.0f / ( history->averageFrameDurationNs / (float)( ONE_NANOSEC ) ) ) );
}


u64 fpscounter_elapsed_time_ns( struct FPSCounter *fpsCounter )
{
    struct FPSHistory *history = &fpsCounter->history;
    return history->lastFramesDurationNs[history->index];
}
