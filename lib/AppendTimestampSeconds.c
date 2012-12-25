/* postgresql: src/backend/utils/adt/datetime.c */

#include "datizo.h"

/* Variant of above that's specialized to timestamp case */
void
AppendTimestampSeconds(char *cp, struct tm * tm, fsec_t fsec)
{
	/*
	 * In float mode, don't print fractional seconds before 1 AD, since it's
	 * unlikely there's any precision left ...
	 */
#ifndef HAVE_INT64_TIMESTAMP
	if (tm->tm_year <= 0)
		fsec = 0;
#endif
	AppendSeconds(cp, tm->tm_sec, fsec, MAX_TIMESTAMP_PRECISION, true);
}


