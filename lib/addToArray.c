/* postgresql: src/backend/utils/misc/tzparser.c */

#include "datizo.h"

#include <stdlib.h>
#include <string.h>

/*
 */
int
addToArray(tzEntry **base, int *arraysize, int n, tzEntry *entry, bool override)
{
	tzEntry    *arrayptr;
	int			low;
	int			high;

	/*
	 * Search the array for a duplicate; as a useful side effect, the array is
	 * maintained in sorted order.	We use strcmp() to ensure we match the
	 * sort order datetime.c expects.
	 */
	arrayptr = *base;
	low = 0;
	high = n - 1;
	while (low <= high)
	{
		int			mid = (low + high) >> 1;
		tzEntry    *midptr = arrayptr + mid;
		int			cmp;

		cmp = strcmp(entry->abbrev, midptr->abbrev);
		if (cmp < 0)
			high = mid - 1;
		else if (cmp > 0)
			low = mid + 1;
		else
		{
			/*
			 * Found a duplicate entry; complain unless it's the same.
			 */
			if (midptr->offset == entry->offset &&
				midptr->is_dst == entry->is_dst)
			{
				/* return unchanged array */
				return n;
			}
			if (override)
			{
				/* same abbrev but something is different, override */
				midptr->offset = entry->offset;
				midptr->is_dst = entry->is_dst;
				return n;
			}
			/* same abbrev but something is different, complain */
			warnx("time zone abbreviation \"%s\" is multiply defined",
							 entry->abbrev);
			warnx("Entry in time zone file \"%s\", line %d, conflicts with entry in file \"%s\", line %d.",
								midptr->filename, midptr->lineno,
								entry->filename, entry->lineno);
			return -1;
		}
	}

	/*
	 * No match, insert at position "low".
	 */
	if (n >= *arraysize)
	{
		*arraysize *= 2;
		*base = (tzEntry *) realloc(*base, *arraysize * sizeof(tzEntry));
	}

	arrayptr = *base + low;

	memmove(arrayptr + 1, arrayptr, (n - low) * sizeof(tzEntry));

	memcpy(arrayptr, entry, sizeof(tzEntry));

	/* Must dup the abbrev to ensure it survives */
	arrayptr->abbrev = strdup(entry->abbrev);

	return n + 1;
}

