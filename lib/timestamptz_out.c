/* postgresql: src/backend/utils/adt/timestamp.c */

#include <string.h>

#include "datizo.h"

/* timestamptz_out()
 * Convert a timestamp to external form.
 */
char *
timestamptz_out(TimestampTz dt)
{
	/*TimestampTz dt = PG_GETARG_TIMESTAMPTZ(0);*/
	char	   *result;
	int			tz;
	struct tm tt,
			   *tm = &tt;
	fsec_t		fsec;
	const char *tzn;
	char		buf[MAXDATELEN + 1];

	if (TIMESTAMP_NOT_FINITE(dt))
		EncodeSpecialTimestamp(dt, buf);
	else if (timestamp2tm(dt, &tz, tm, &fsec, &tzn, NULL) == 0)
		EncodeDateTime(tm, fsec, true, tz, tzn, DateStyle, buf);
	else
		warnx("timestamp out of range");

	result = strdup(buf);

	return result;
}

