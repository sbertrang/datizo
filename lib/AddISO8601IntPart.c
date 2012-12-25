/* postgresql: src/backend/utils/adt/datetime.c */

#include <stdio.h>
#include <string.h>

#include "datizo.h"


/* Append an ISO-8601-style interval field, but only if value isn't zero */
char *
AddISO8601IntPart(char *cp, int value, char units)
{
	if (value == 0)
		return cp;
	sprintf(cp, "%d%c", value, units);
	return cp + strlen(cp);
}


