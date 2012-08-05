
#include "datizo.h"

/*
 * GetCurrentDateTime()
 *
 * Get the transaction start time ("now()") broken down as a struct pg_tm.
 */
void
GetCurrentDateTime(struct tm * tm)
{
	int			tz;
	fsec_t		fsec;

	timestamp2tm(GetCurrentTransactionStartTimestamp(), &tz, tm, &fsec,
				 NULL, NULL);
	/* Note: don't pass NULL tzp to timestamp2tm; affects behavior */
}

