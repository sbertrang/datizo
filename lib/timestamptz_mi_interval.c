
/*
 * src/backend/utils/adt/timestamp.c
 */

#include "datizo.h"

TimestampTz
timestamptz_mi_interval(TimestampTz timestamp, Interval *span)
{
/*
	TimestampTz timestamp = PG_GETARG_TIMESTAMPTZ(0);
	Interval   *span = PG_GETARG_INTERVAL_P(1);
*/
	Interval	tspan;

	tspan.month = -span->month;
	tspan.day = -span->day;
	tspan.time = -span->time;

	return timestamptz_pl_interval(timestamp, &tspan);
}


