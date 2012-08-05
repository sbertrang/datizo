
#include <time.h>

#include "datizo.h"

/*
 * Section 4.12.3 of X3.159-1989 requires that
 *      Except for the strftime function, these functions [asctime,
 *      ctime, gmtime, localtime] return values in one of two static
 *      objects: a broken-down time structure and an array of char.
 * Thanks to Paul Eggert for noting this.
 */

static struct tm tm;

struct tm *
pg_localtime(const pg_time_t *timep, const pg_tz *tz)
{
	return localsub(timep, 0L, &tm, tz);
}


