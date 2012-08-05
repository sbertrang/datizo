
#include "datizo.h"

int
tm2interval(struct tm * tm, fsec_t fsec, Interval *span)
{
	span->month = tm->tm_year * MONTHS_PER_YEAR + tm->tm_mon;
	span->day = tm->tm_mday;
#ifdef HAVE_INT64_TIMESTAMP
	span->time = (((((tm->tm_hour * INT64CONST(60)) +
					 tm->tm_min) * INT64CONST(60)) +
				   tm->tm_sec) * USECS_PER_SEC) + fsec;
#else
	span->time = (((tm->tm_hour * (double) MINS_PER_HOUR) +
				   tm->tm_min) * (double) SECS_PER_MINUTE) +
		tm->tm_sec + fsec;
#endif

	return 0;
}

