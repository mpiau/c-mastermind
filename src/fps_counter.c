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
    FRAME_HISTORY_COUNT = 8,

    FRAMERATE_90  = NANOSECONDS / 90,
    FRAMERATE_75  = NANOSECONDS / 75,
    FRAMERATE_60  = NANOSECONDS / 60,
    FRAMERATE_45  = NANOSECONDS / 45,
    FRAMERATE_30  = NANOSECONDS / 30
};

struct FrameHistory
{
    nanoseconds frameDuration[FRAME_HISTORY_COUNT];
    nanoseconds totalDuration;
    nanoseconds averageDuration;
    u8  frameIndex;
};

struct FPSCounter
{
    HANDLE waitableTimer;
    LARGE_INTEGER minWaitTimePerFrame100ns;

    nanoseconds frameBegin;
    nanoseconds frameEnd;

    struct FrameHistory history;
};

static struct FPSCounter s_fpsCounter = {}; // Just to avoid dynamic alloc

static nanoseconds S_CAPPED_FRAMERATE = FRAMERATE_60;


nanoseconds get_timestamp_nanoseconds()
{
    struct timespec time;
    clock_gettime( CLOCK_MONOTONIC, &time );
    return time.tv_sec * NANOSECONDS + time.tv_nsec;
}

struct FPSCounter *fpscounter_init( void )
{
    struct FPSCounter *fpsCounter = &s_fpsCounter;
    fpsCounter->waitableTimer = CreateWaitableTimerExW( NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS );
    if ( fpsCounter->waitableTimer == NULL || fpsCounter->waitableTimer == INVALID_HANDLE_VALUE )
    {
        return NULL;
    }

    // - 1 ms for accuracy, otherwise we will be at 59fps instead of 60fps
    // negative to be relative and not UTC.
    fpsCounter->minWaitTimePerFrame100ns.QuadPart = (i64)( (u64)( S_CAPPED_FRAMERATE - 1000000 ) / (u64)100 ) * -1;

    SetWaitableTimerEx( fpsCounter->waitableTimer, &fpsCounter->minWaitTimePerFrame100ns, 0, NULL, NULL, NULL, 0 );
    fpsCounter->frameBegin = get_timestamp_nanoseconds();

    return fpsCounter;
}


struct FPSCounter *fpscounter_get_instance( void )
{
	return &s_fpsCounter;
}


void fpscounter_uninit( struct FPSCounter *fpsCounter )
{
    // Note: Not sure we should export FPSCounter as we only should get only one in the entire game.
    if ( fpsCounter && fpsCounter->waitableTimer != INVALID_HANDLE_VALUE )
    {
        CloseHandle( fpsCounter->waitableTimer );
    }
}


u64 fpscounter_average_framerate( struct FPSCounter *fpsCounter )
{
    struct FrameHistory *history = &fpsCounter->history;
    return (u64)( roundf( 1.0f / ( history->averageDuration / (float)( NANOSECONDS ) ) ) );
}


u64 fpscounter_frame( struct FPSCounter *fpsCounter )
{
    WaitForSingleObject( fpsCounter->waitableTimer, INFINITE );

    do
    {
        fpsCounter->frameEnd = get_timestamp_nanoseconds();
    } while ( fpsCounter->frameEnd - fpsCounter->frameBegin < S_CAPPED_FRAMERATE );

	nanoseconds const delta = fpsCounter->frameEnd - fpsCounter->frameBegin;

    struct FrameHistory *history = &fpsCounter->history;
    history->totalDuration -= history->frameDuration[history->frameIndex];
    history->totalDuration += delta;
    history->frameDuration[history->frameIndex] = delta;
    history->frameIndex = ( history->frameIndex + 1 ) % FRAME_HISTORY_COUNT;
    history->averageDuration = history->totalDuration / FRAME_HISTORY_COUNT;

    // Prepare the next frame
    SetWaitableTimerEx( fpsCounter->waitableTimer, &fpsCounter->minWaitTimePerFrame100ns, 0, NULL, NULL, NULL, 0 );
	fpsCounter->frameBegin = fpsCounter->frameEnd;
}



u64 fpscounter_elapsed_time( struct FPSCounter *fpsCounter )
{
    struct FrameHistory *history = &fpsCounter->history;
    return history->frameDuration[history->frameIndex];
}

u64 fpscounter_average_time( struct FPSCounter *fpsCounter )
{
    struct FrameHistory *history = &fpsCounter->history;
    return history->averageDuration;
}
