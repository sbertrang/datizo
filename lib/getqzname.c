
#include "datizo.h"

/*
 * Given a pointer into an extended time zone string, scan until the ending
 * delimiter of the zone name is located. Return a pointer to the delimiter.
 *
 * As with getzname above, the legal character set is actually quite
 * restricted, with other characters producing undefined results.
 * We don't do any checking here; checking is done later in common-case code.
 */
const char *
getqzname(const char *strp, int delim)
{
	int			c;

	while ((c = *strp) != '\0' && c != delim)
		++strp;
	return strp;
}


