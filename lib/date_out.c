/* postgresql: src/backend/utils/adt/date.c */

#include <string.h>

#include "datizo.h"

/* date_out()
 * Given internal format date, convert to text string.
 */
char *
date_out(DateADT date)
{
	/*DateADT		date = PG_GETARG_DATEADT(0);*/
	char	   *result;
	struct tm tt,
			   *tm = &tt;
	char		buf[MAXDATELEN + 1];

	if (DATE_NOT_FINITE(date))
		EncodeSpecialDate(date, buf);
	else
	{
		j2date(date + POSTGRES_EPOCH_JDATE,
			   &(tm->tm_year), &(tm->tm_mon), &(tm->tm_mday));
		EncodeDateOnly(tm, DateStyle, buf);
	}

	result = strdup(buf);
	return result;
}

