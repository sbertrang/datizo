
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "datizo.h"

/* Fetch a fractional-second value with suitable error checking */
int
ParseFractionalSecond(char *cp, fsec_t *fsec)
{
	double		frac;

	/* Caller should always pass the start of the fraction part */
	assert(*cp == '.');
	errno = 0;
	frac = strtod(cp, &cp);
	/* check for parse failure */
	if (*cp != '\0' || errno != 0)
		return DTERR_BAD_FORMAT;
#ifdef HAVE_INT64_TIMESTAMP
	*fsec = rint(frac * 1000000);
#else
	*fsec = frac;
#endif
	return 0;
}


