
#include "datizo.h"

/*
 * If the given timezone uses only one GMT offset, store that offset
 * into *gmtoff and return TRUE, else return FALSE.
 */
bool
pg_get_timezone_offset(const pg_tz *tz, long int *gmtoff)
{
	/*
	 * The zone could have more than one ttinfo, if it's historically used
	 * more than one abbreviation.	We return TRUE as long as they all have
	 * the same gmtoff.
	 */
	const struct state *sp;
	int			i;

	sp = &tz->state;
	for (i = 1; i < sp->typecnt; i++)
	{
		if (sp->ttis[i].tt_gmtoff != sp->ttis[0].tt_gmtoff)
			return false;
	}
	*gmtoff = sp->ttis[0].tt_gmtoff;
	return true;
}

