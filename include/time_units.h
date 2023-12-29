#pragma once

#include "core_types.h"

typedef u64 nsec;
typedef u64 usec;
typedef u64 msec;
typedef u64 sec;
typedef u64 min;
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


nsec time_get_timestamp_nsec( void );


static inline nsec time_msec_to_nsec( msec const milliseconds )
{
	return milliseconds * Time_MSEC_IN_NSEC;
}

static inline msec time_nsec_to_msec( nsec const nanoseconds )
{
	return nanoseconds / Time_MSEC_IN_NSEC;
}


static inline nsec time_sec_to_nsec( sec const seconds )
{
	return seconds * Time_SEC_IN_NSEC;
}

static inline sec time_nsec_to_sec( nsec const nanoseconds )
{
	return nanoseconds / Time_SEC_IN_NSEC;
}
