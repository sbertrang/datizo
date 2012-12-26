/* postgresql: src/backend/utils/adt/date.c */

#include <string.h>

#include "datizo.h"
char *
timetz_out(TimeTzADT *time)
{
	char	   *result;
	struct tm tt,
			   *tm = &tt;
	fsec_t		fsec;
	int			tz;
	char		buf[MAXDATELEN + 1];

	timetz2tm(time, tm, &fsec, &tz);
	EncodeTimeOnly(tm, fsec, true, tz, DateStyle, buf);

	result = strdup(buf);
	return result;
}

