
#include <string.h>

#include "datizo.h"

char *
time_out(TimeADT time)
{
	/*TimeADT		time = PG_GETARG_TIMEADT(0);*/
	char	   *result;
	struct tm tt,
			   *tm = &tt;
	fsec_t		fsec;
	char		buf[MAXDATELEN + 1];

	time2tm(time, tm, &fsec);
	EncodeTimeOnly(tm, fsec, false, 0, DateStyle, buf);

	result = strdup(buf);
	return result;
}


