
#include "datizo.h"

/*
 * GetCurrentTimeUsec()
 *
 * Get the transaction start time ("now()") broken down as a struct pg_tm,
 * including fractional seconds and timezone offset.
 */
void
GetCurrentTimeUsec(struct tm * tm, fsec_t *fsec, int *tzp)
{
	int			tz;

	timestamp2tm(GetCurrentTransactionStartTimestamp(), &tz, tm, fsec,
				 NULL, NULL);
	/* Note: don't pass NULL tzp to timestamp2tm; affects behavior */
	if (tzp != NULL)
		*tzp = tz;
}

