#include "time_units.h"

#include <time.h>

nsecond time_get_timestamp_nsec( void )
{
    struct timespec time;
    clock_gettime( CLOCK_MONOTONIC, &time );

    return time_sec_to_nsec( time.tv_sec ) + time.tv_nsec;
}
