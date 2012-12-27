/* postgresql: src/backend/utils/adt/timestamp.c */

#include "datizo.h"

/*
 * We are currently sharing some code between timestamp and timestamptz.
 * The comparison functions are among them. - thomas 2001-09-25
 *
 *		timestamp_relop - is timestamp1 relop timestamp2
 *
 *		collate invalid timestamp at the end
 */

int
timestamp_cmp_internal(Timestamp dt1, Timestamp dt2)
{
#ifdef HAVE_INT64_TIMESTAMP
	return (dt1 < dt2) ? -1 : ((dt1 > dt2) ? 1 : 0);
#else

	/*
	 * When using float representation, we have to be wary of NaNs.
	 *
	 * We consider all NANs to be equal and larger than any non-NAN. This is
	 * somewhat arbitrary; the important thing is to have a consistent sort
	 * order.
	 */
	if (isnan(dt1))
	{
		if (isnan(dt2))
			return 0;			/* NAN = NAN */
		else
			return 1;			/* NAN > non-NAN */
	}
	else if (isnan(dt2))
	{
		return -1;				/* non-NAN < NAN */
	}
	else
	{
		if (dt1 > dt2)
			return 1;
		else if (dt1 < dt2)
			return -1;
		else
			return 0;
	}
#endif
}

