#pragma once

#include "core_types.h"

typedef u64 nanoseconds;
typedef u64 microseconds;
typedef u64 milliseconds;
typedef u64 seconds;
typedef u64 minutes;
typedef u64 hours;

enum
{
    SECOND       = 1, // Not clear enough, because SECOND could be different depending of the referentiel ( SECOND would be 1000000000 on nanosec )
    MILLISECONDS = SECOND       * 1000,
    MICROSECONDS = MILLISECONDS * 1000,
    NANOSECONDS  = MICROSECONDS * 1000
};


static inline nanoseconds seconds_to_nanoseconds( seconds const s )
{
    return s * NANOSECONDS;
}

static inline milliseconds seconds_to_milliseconds( seconds const s )
{
    return s * MILLISECONDS;
}


static inline seconds milliseconds_to_seconds( milliseconds const ms )
{
    return ms / MILLISECONDS;
}

static inline nanoseconds milliseconds_to_nanoseconds( milliseconds const ms )
{
    return ms * ( NANOSECONDS / MILLISECONDS );
}


static inline seconds nanoseconds_to_seconds( nanoseconds const ns )
{
    return ns / NANOSECONDS;
}

static inline milliseconds nanoseconds_to_milliseconds( nanoseconds const ns )
{
    return ns / ( NANOSECONDS / MILLISECONDS );
}
