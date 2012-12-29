/* postgresql: src/backend/utils/misc/tzparser.c */

#include "datizo.h"
#include <string.h>

/*
 */
bool
validateTzEntry(tzEntry *tzentry)
{
	unsigned char *p;

	/*
	 * Check restrictions imposed by datetkntbl storage format (see
	 * datetime.c)
	 */
	if (strlen(tzentry->abbrev) > TOKMAXLEN)
	{
		warnx("time zone abbreviation \"%s\" is too long (maximum %d characters) in time zone file \"%s\", line %d",
						 tzentry->abbrev, TOKMAXLEN,
						 tzentry->filename, tzentry->lineno);
		return false;
	}
	if (tzentry->offset % 900 != 0)
	{
		warnx("time zone offset %d is not a multiple of 900 sec (15 min) in time zone file \"%s\", line %d",
						 tzentry->offset,
						 tzentry->filename, tzentry->lineno);
		return false;
	}

	/*
	 * Sanity-check the offset: shouldn't exceed 14 hours
	 */
	if (tzentry->offset > 14 * 60 * 60 ||
		tzentry->offset < -14 * 60 * 60)
	{
		warnx("time zone offset %d is out of range in time zone file \"%s\", line %d",
						 tzentry->offset,
						 tzentry->filename, tzentry->lineno);
		return false;
	}

	/*
	 * Convert abbrev to lowercase (must match datetime.c's conversion)
	 */
	for (p = (unsigned char *) tzentry->abbrev; *p; p++)
		*p = pg_tolower(*p);

	return true;
}

