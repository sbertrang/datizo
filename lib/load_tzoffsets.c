/* postgresql: src/backend/utils/misc/tzparser.c */

#include "datizo.h"

/*
 */
TimeZoneAbbrevTable *
load_tzoffsets(const char *filename)
{
	TimeZoneAbbrevTable *result = NULL;
#if 0
	MemoryContext tmpContext;
	MemoryContext oldContext;
#endif
	tzEntry    *array;
	int			arraysize;
	int			n;

	/*
	 * Create a temp memory context to work in.  This makes it easy to clean
	 * up afterwards.
	 */
#if 0
	tmpContext = AllocSetContextCreate(CurrentMemoryContext,
									   "TZParserMemory",
									   ALLOCSET_SMALL_MINSIZE,
									   ALLOCSET_SMALL_INITSIZE,
									   ALLOCSET_SMALL_MAXSIZE);
	oldContext = MemoryContextSwitchTo(tmpContext);
#endif

	/* Initialize array at a reasonable size */
	arraysize = 128;
	array = (tzEntry *) malloc(arraysize * sizeof(tzEntry));

	/* Parse the file(s) */
	n = ParseTzFile(filename, 0, &array, &arraysize, 0);

	/* If no errors so far, allocate result and let datetime.c convert data */
	if (n >= 0)
	{
		result = malloc(offsetof(TimeZoneAbbrevTable, abbrevs) +
						n * sizeof(datetkn));
		if (!result)
			warnx("out of memory");
		else
			ConvertTimeZoneAbbrevs(result, array, n);
	}

	/* Clean up */
#if 0
	MemoryContextSwitchTo(oldContext);
	MemoryContextDelete(tmpContext);
#endif

	return result;
}

