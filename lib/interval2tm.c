/* postgresql: src/backend/utils/adt/timestamp.c */


#include "datizo.h"

/* interval2tm()
 * Convert a interval data type to a tm structure.
 */
int
interval2tm(Interval span, struct tm * tm, fsec_t *fsec)
{
	TimeOffset	time;
	TimeOffset	tfrac;

	tm->tm_year = span.month / MONTHS_PER_YEAR;
	tm->tm_mon = span.month % MONTHS_PER_YEAR;
	tm->tm_mday = span.day;
	time = span.time;

#ifdef HAVE_INT64_TIMESTAMP
	tfrac = time / USECS_PER_HOUR;
	time -= tfrac * USECS_PER_HOUR;
	tm->tm_hour = tfrac;		/* could overflow ... */
	tfrac = time / USECS_PER_MINUTE;
	time -= tfrac * USECS_PER_MINUTE;
	tm->tm_min = tfrac;
	tfrac = time / USECS_PER_SEC;
	*fsec = time - (tfrac * USECS_PER_SEC);
	tm->tm_sec = tfrac;
#else
recalc:
	TMODULO(time, tfrac, (double) SECS_PER_HOUR);
	tm->tm_hour = tfrac;		/* could overflow ... */
	TMODULO(time, tfrac, (double) SECS_PER_MINUTE);
	tm->tm_min = tfrac;
	TMODULO(time, tfrac, 1.0);
	tm->tm_sec = tfrac;
	time = TSROUND(time);
	/* roundoff may need to propagate to higher-order fields */
	if (time >= 1.0)
	{
		time = ceil(span.time);
		goto recalc;
	}
	*fsec = time;
#endif

	return 0;
}


