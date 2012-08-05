

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datizo.h"

/* EncodeTimezone()
 *		Append representation of a numeric timezone offset to str.
 */
void
EncodeTimezone(char *str, int tz, int style)
{
	int			hour,
				min,
				sec;

	sec = abs(tz);
	min = sec / SECS_PER_MINUTE;
	sec -= min * SECS_PER_MINUTE;
	hour = min / MINS_PER_HOUR;
	min -= hour * MINS_PER_HOUR;

	str += strlen(str);
	/* TZ is negated compared to sign we wish to display ... */
	*str++ = (tz <= 0 ? '+' : '-');

	if (sec != 0)
		sprintf(str, "%02d:%02d:%02d", hour, min, sec);
	else if (min != 0 || style == USE_XSD_DATES)
		sprintf(str, "%02d:%02d", hour, min);
	else
		sprintf(str, "%02d", hour);
}

