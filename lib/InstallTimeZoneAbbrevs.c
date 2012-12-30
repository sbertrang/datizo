/* postgresql: src/backend/utils/adt/datetime.c */

#include "datizo.h"

/*
 */
void
InstallTimeZoneAbbrevs(TimeZoneAbbrevTable *tbl)
{
	int			i;

	timezonetktbl = tbl->abbrevs;
	sztimezonetktbl = tbl->numabbrevs;

	/* clear date cache in case it contains any stale timezone names */
	for (i = 0; i < MAXDATEFIELDS; i++)
		datecache[i] = NULL;
}
