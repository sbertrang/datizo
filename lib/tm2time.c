
#include "datizo.h"

/* tm2time()
 * Convert a tm structure to a time data type.
 */
int
tm2time(struct tm * tm, fsec_t fsec, TimeADT *result)
{
#ifdef HAVE_INT64_TIMESTAMP
	*result = ((((tm->tm_hour * MINS_PER_HOUR + tm->tm_min) * SECS_PER_MINUTE) + tm->tm_sec)
			   * USECS_PER_SEC) + fsec;
#else
	*result = ((tm->tm_hour * MINS_PER_HOUR + tm->tm_min) * SECS_PER_MINUTE) + tm->tm_sec + fsec;
#endif
	return 0;
}

