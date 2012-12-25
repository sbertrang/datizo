/* postgresql: src/backend/utils/adt/datetime.c */

#include <stdio.h>
#include <string.h>

#include "datizo.h"

/* EncodeTimeOnly()
 * Encode time fields only.
 *
 * tm and fsec are the value to encode, print_tz determines whether to include
 * a time zone (the difference between time and timetz types), tz is the
 * numeric time zone offset, style is the date style, str is where to write the
 * output.
 */
void
EncodeTimeOnly(struct tm * tm, fsec_t fsec, bool print_tz, int tz, int style, char *str)
{
	sprintf(str, "%02d:%02d:", tm->tm_hour, tm->tm_min);
	str += strlen(str);

	AppendSeconds(str, tm->tm_sec, fsec, MAX_TIME_PRECISION, true);

	if (print_tz)
		EncodeTimezone(str, tz, style);
}

