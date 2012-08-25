
#include "datizo.h"

TimeOffset
time2t(const int hour, const int min, const int sec, const fsec_t fsec)
{
#ifdef HAVE_INT64_TIMESTAMP
	return (((((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec) * USECS_PER_SEC) + fsec;
#else
	return (((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec + fsec;
#endif
}

