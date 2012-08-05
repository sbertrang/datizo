
#include "datizo.h"

Timestamp
SetEpochTimestamp(void)
{
	Timestamp	dt;
	struct tm tt,
			   *tm = &tt;

	GetEpochTime(tm);
	/* we don't bother to test for failure ... */
	tm2timestamp(tm, 0, NULL, &dt);

	return dt;
}	/* SetEpochTimestamp() */

