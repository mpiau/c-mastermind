#pragma once

#include "core_types.h"

typedef u64 nsecond;
typedef u64 usecond;
typedef u64 msecond;
typedef u64 second;
typedef u64 minute;
typedef u64 hour;
typedef u64 day;

enum
{
	Time_SEC_IN_MSEC = 1000,
	Time_SEC_IN_USEC = Time_SEC_IN_MSEC * 1000,
	Time_SEC_IN_NSEC = Time_SEC_IN_USEC * 1000,

	Time_MSEC_IN_USEC = 1000,
	Time_MSEC_IN_NSEC = Time_MSEC_IN_USEC * 1000,

	Time_USEC_IN_NSEC = 1000,
};


nsecond time_get_timestamp_nsec( void );


static inline nsecond time_msec_to_nsec( msecond const milliseconds )
{
	return milliseconds * Time_MSEC_IN_NSEC;
}

static inline msecond time_nsec_to_msec( nsecond const nanoseconds )
{
	return nanoseconds / Time_MSEC_IN_NSEC;
}


static inline nsecond time_sec_to_nsec( second const seconds )
{
	return seconds * Time_SEC_IN_NSEC;
}

static inline second time_nsec_to_sec( nsecond const nanoseconds )
{
	return nanoseconds / Time_SEC_IN_NSEC;
}
