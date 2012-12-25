/* postgresql: src/backend/utils/adt/date.c */

#include "datizo.h"

/* tm2timetz()
 * Convert a tm structure to a time data type.
 */
int
tm2timetz(struct tm * tm, fsec_t fsec, int tz, TimeTzADT *result)
{
#ifdef HAVE_INT64_TIMESTAMP
	result->time = ((((tm->tm_hour * MINS_PER_HOUR + tm->tm_min) * SECS_PER_MINUTE) + tm->tm_sec) *
					USECS_PER_SEC) + fsec;
#else
	result->time = ((tm->tm_hour * MINS_PER_HOUR + tm->tm_min) * SECS_PER_MINUTE) + tm->tm_sec + fsec;
#endif
	result->zone = tz;

	return 0;
}

