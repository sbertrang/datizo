
#include <stdlib.h>

#include "datizo.h"

/*
 *	interval_justify_interval()
 *
 *	Adjust interval so 'month', 'day', and 'time' portions are within
 *	customary bounds.  Specifically:
 *
 *		0 <= abs(time) < 24 hours
 *		0 <= abs(day)  < 30 days
 *
 *	Also, the sign bit on all three fields is made equal, so either
 *	all three fields are negative or all are positive.
 */
Interval *
interval_justify_interval(Interval *span)
{
/*
	Interval   *span = PG_GETARG_INTERVAL_P(0);
*/
	Interval   *result;
	TimeOffset	wholeday;
	int32_t		wholemonth;

	result = (Interval *) malloc(sizeof(Interval));
	result->month = span->month;
	result->day = span->day;
	result->time = span->time;

#ifdef HAVE_INT64_TIMESTAMP
	TMODULO(result->time, wholeday, USECS_PER_DAY);
#else
	TMODULO(result->time, wholeday, (double) SECS_PER_DAY);
#endif
	result->day += wholeday;	/* could overflow... */

	wholemonth = result->day / DAYS_PER_MONTH;
	result->day -= wholemonth * DAYS_PER_MONTH;
	result->month += wholemonth;

	if (result->month > 0 &&
		(result->day < 0 || (result->day == 0 && result->time < 0)))
	{
		result->day += DAYS_PER_MONTH;
		result->month--;
	}
	else if (result->month < 0 &&
			 (result->day > 0 || (result->day == 0 && result->time > 0)))
	{
		result->day -= DAYS_PER_MONTH;
		result->month++;
	}

	if (result->day > 0 && result->time < 0)
	{
#ifdef HAVE_INT64_TIMESTAMP
		result->time += USECS_PER_DAY;
#else
		result->time += (double) SECS_PER_DAY;
#endif
		result->day--;
	}
	else if (result->day < 0 && result->time > 0)
	{
#ifdef HAVE_INT64_TIMESTAMP
		result->time -= USECS_PER_DAY;
#else
		result->time -= (double) SECS_PER_DAY;
#endif
		result->day++;
	}

	return result;
}


