
#include <stdlib.h>

#include "datizo.h"

/*
 *	interval_justify_days()
 *
 *	Adjust interval so 'day' contains less than 30 days, adding
 *	the excess to 'month'.
 */
Interval *
interval_justify_days(Interval *span)
{
/*
	Interval   *span = PG_GETARG_INTERVAL_P(0);
*/
	Interval   *result;
	int32_t		wholemonth;

	result = (Interval *) malloc(sizeof(Interval));
	result->month = span->month;
	result->day = span->day;
	result->time = span->time;

	wholemonth = result->day / DAYS_PER_MONTH;
	result->day -= wholemonth * DAYS_PER_MONTH;
	result->month += wholemonth;

	if (result->month > 0 && result->day < 0)
	{
		result->day += DAYS_PER_MONTH;
		result->month--;
	}
	else if (result->month < 0 && result->day > 0)
	{
		result->day -= DAYS_PER_MONTH;
		result->month++;
	}

	return result;
}

