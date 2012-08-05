
#include <time.h>

#include "datizo.h"

/* GMT timezone */
static struct state gmtmem;

#define gmtptr          (&gmtmem)


static int      gmt_is_set = 0;

/*
 * gmtsub is to gmtime as localsub is to localtime.
 */
struct tm *
gmtsub(const pg_time_t *timep, long offset, struct tm * tmp)
{
	struct tm *result;

	if (!gmt_is_set)
	{
		gmt_is_set = TRUE;
		gmtload(gmtptr);
	}
	result = timesub(timep, offset, gmtptr, tmp);

	/*
	 * Could get fancy here and deliver something such as "UTC+xxxx" or
	 * "UTC-xxxx" if offset is non-zero, but this is no time for a treasure
	 * hunt.
	 */
	if (offset != 0)
		tmp->tm_zone = wildabbr;
	else
		tmp->tm_zone = gmtptr->chars;

	return result;
}

