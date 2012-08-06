
#include "datizo.h"

/* timetz2tm()
 * Convert TIME WITH TIME ZONE data type to POSIX time structure.
 */
int
timetz2tm(TimeTzADT *time, struct tm * tm, fsec_t *fsec, int *tzp)
{
	TimeOffset	trem = time->time;

#ifdef HAVE_INT64_TIMESTAMP
	tm->tm_hour = trem / USECS_PER_HOUR;
	trem -= tm->tm_hour * USECS_PER_HOUR;
	tm->tm_min = trem / USECS_PER_MINUTE;
	trem -= tm->tm_min * USECS_PER_MINUTE;
	tm->tm_sec = trem / USECS_PER_SEC;
	*fsec = trem - tm->tm_sec * USECS_PER_SEC;
#else
recalc:
	TMODULO(trem, tm->tm_hour, (double) SECS_PER_HOUR);
	TMODULO(trem, tm->tm_min, (double) SECS_PER_MINUTE);
	TMODULO(trem, tm->tm_sec, 1.0);
	trem = TIMEROUND(trem);
	/* roundoff may need to propagate to higher-order fields */
	if (trem >= 1.0)
	{
		trem = ceil(time->time);
		goto recalc;
	}
	*fsec = trem;
#endif

	if (tzp != NULL)
		*tzp = time->zone;

	return 0;
}

