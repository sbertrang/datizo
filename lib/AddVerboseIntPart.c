
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datizo.h"

/* Append a verbose-style interval field, but only if value isn't zero */
char *
AddVerboseIntPart(char *cp, int value, const char *units,
				  bool *is_zero, bool *is_before)
{
	if (value == 0)
		return cp;
	/* first nonzero value sets is_before */
	if (*is_zero)
	{
		*is_before = (value < 0);
		value = abs(value);
	}
	else if (*is_before)
		value = -value;
	sprintf(cp, " %d %s%s", value, units, (value == 1) ? "" : "s");
	*is_zero = FALSE;
	return cp + strlen(cp);
}



