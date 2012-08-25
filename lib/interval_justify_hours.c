
#include <stdlib.h>

#include "datizo.h"

/*
 *	interval_justify_hours()
 *
 *	Adjust interval so 'time' contains less than a whole day, adding
 *	the excess to 'day'.  This is useful for
 *	situations (such as non-TZ) where '1 day' = '24 hours' is valid,
 *	e.g. interval subtraction and division.
 */
Interval *
interval_justify_hours(Interval *span)
{
/*
	Interval   *span = PG_GETARG_INTERVAL_P(0);
*/
	Interval   *result;
	TimeOffset	wholeday;

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

