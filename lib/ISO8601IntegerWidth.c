
#include <string.h>

#include "datizo.h"

/*
 * Determine number of integral digits in a valid ISO 8601 number field
 * (we should ignore sign and any fraction part)
 */
int
ISO8601IntegerWidth(char *fieldstart)
{
	/* We might have had a leading '-' */
	if (*fieldstart == '-')
		fieldstart++;
	return strspn(fieldstart, "0123456789");
}


