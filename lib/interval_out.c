/* postgresql: src/backend/utils/adt/timestamp.c */

#include <string.h>

#include "datizo.h"

/* interval_out()
 * Convert a time span to external form.
 */
char *
interval_out(Interval *span)
{
	char	   *result;
	struct tm tt,
			   *tm = &tt;
	fsec_t		fsec;
	char		buf[MAXDATELEN + 1];

	if (interval2tm(*span, tm, &fsec) != 0)
		warnx("could not convert interval to tm");

	EncodeInterval(tm, fsec, IntervalStyle, buf);

	result = strdup(buf);
	return result;
}



