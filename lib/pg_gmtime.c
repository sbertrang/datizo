/* postgresql: src/timezone/localtime.c */

#include "datizo.h"

static struct tm tm;

struct tm *
pg_gmtime(const pg_time_t *timep)
{
	return gmtsub(timep, 0L, &tm);
}

