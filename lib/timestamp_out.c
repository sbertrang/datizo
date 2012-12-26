/* postgresql: src/backend/utils/adt/timestamp.c */

#include <string.h>

#include "datizo.h"

/* timestamp_out()
 * Convert a timestamp to external form.
 */
char *
timestamp_out(Timestamp timestamp)
{
	/* Timestamp	timestamp = PG_GETARG_TIMESTAMP(0); */
	char	   *result;
	struct tm tt,
			   *tm = &tt;
	fsec_t		fsec;
	char		buf[MAXDATELEN + 1];

	if (TIMESTAMP_NOT_FINITE(timestamp))
		EncodeSpecialTimestamp(timestamp, buf);
	else if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) == 0)
		EncodeDateTime(tm, fsec, false, 0, NULL, DateStyle, buf);
	else
		warnx("timestamp out of range");

	result = strdup(buf);

	return result;
}


