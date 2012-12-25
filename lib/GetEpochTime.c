/* postgresql: src/backend/utils/adt/timestamp.c */

#include "datizo.h"

void
GetEpochTime(struct tm * tm)
{
	struct tm *t0;
	pg_time_t	epoch = 0;

	t0 = pg_gmtime(&epoch);

	tm->tm_year = t0->tm_year;
	tm->tm_mon = t0->tm_mon;
	tm->tm_mday = t0->tm_mday;
	tm->tm_hour = t0->tm_hour;
	tm->tm_min = t0->tm_min;
	tm->tm_sec = t0->tm_sec;

	tm->tm_year += 1900;
	tm->tm_mon++;
}

