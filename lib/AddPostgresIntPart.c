/* postgresql: src/backend/utils/adt/datetime.c */

#include <stdio.h>
#include <string.h>

#include "datizo.h"

/* Append a postgres-style interval field, but only if value isn't zero */
char *
AddPostgresIntPart(char *cp, int value, const char *units,
				   bool *is_zero, bool *is_before)
{
	if (value == 0)
		return cp;
	sprintf(cp, "%s%s%d %s%s",
			(!*is_zero) ? " " : "",
			(*is_before && value > 0) ? "+" : "",
			value,
			units,
			(value != 1) ? "s" : "");

	/*
	 * Each nonzero field sets is_before for (only) the next one.  This is a
	 * tad bizarre but it's how it worked before...
	 */
	*is_before = (value < 0);
	*is_zero = FALSE;
	return cp + strlen(cp);
}


