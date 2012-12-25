/* postgresql: src/backend/utils/adt/date.c */

#include <string.h>

#include "datizo.h"

/*
 * Convert reserved date values to string.
 */
void
EncodeSpecialDate(DateADT dt, char *str)
{
	if (DATE_IS_NOBEGIN(dt))
		strcpy(str, EARLY);
	else if (DATE_IS_NOEND(dt))
		strcpy(str, LATE);
	else	/* shouldn't happen */
		warnx("invalid argument for EncodeSpecialDate");
}

