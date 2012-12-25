/* postgresql: src/backend/utils/adt/date.c */

#include "datizo.h"

/* time2tm()
 * Convert time data type to POSIX time structure.
 *
 * For dates within the range of pg_time_t, convert to the local time zone.
 * If out of this range, leave as UTC (in practice that could only happen
 * if pg_time_t is just 32 bits) - thomas 97/05/27
 */
int
time2tm(TimeADT time, struct tm * tm, fsec_t *fsec)
{
#ifdef HAVE_INT64_TIMESTAMP
	tm->tm_hour = time / USECS_PER_HOUR;
	time -= tm->tm_hour * USECS_PER_HOUR;
	tm->tm_min = time / USECS_PER_MINUTE;
	time -= tm->tm_min * USECS_PER_MINUTE;
	tm->tm_sec = time / USECS_PER_SEC;
	time -= tm->tm_sec * USECS_PER_SEC;
	*fsec = time;
#else
	double		trem;

recalc:
	trem = time;
	TMODULO(trem, tm->tm_hour, (double) SECS_PER_HOUR);
	TMODULO(trem, tm->tm_min, (double) SECS_PER_MINUTE);
	TMODULO(trem, tm->tm_sec, 1.0);
	trem = TIMEROUND(trem);
	/* roundoff may need to propagate to higher-order fields */
	if (trem >= 1.0)
	{
		time = ceil(time);
		goto recalc;
	}
	*fsec = trem;
#endif

	return 0;
}

