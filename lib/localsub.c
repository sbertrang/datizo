
#include <time.h>

#include "datizo.h"

/*
 * The easy way to behave "as if no library function calls" localtime
 * is to not call it--so we drop its guts into "localsub", which can be
 * freely called. (And no, the PANS doesn't require the above behavior--
 * but it *is* desirable.)
 *
 * The unused offset argument is for the benefit of mktime variants.
 */
struct tm *
localsub(const pg_time_t *timep, long offset,
		 struct tm * tmp, const pg_tz *tz)
{
	const struct state *sp;
	const struct ttinfo *ttisp;
	int			i;
	struct tm *result;
	const pg_time_t t = *timep;

	sp = &tz->state;
	if ((sp->goback && t < sp->ats[0]) ||
		(sp->goahead && t > sp->ats[sp->timecnt - 1]))
	{
		pg_time_t	newt = t;
		pg_time_t	seconds;
		pg_time_t	tcycles;
		int64_t		icycles;

		if (t < sp->ats[0])
			seconds = sp->ats[0] - t;
		else
			seconds = t - sp->ats[sp->timecnt - 1];
		--seconds;
		tcycles = seconds / YEARSPERREPEAT / AVGSECSPERYEAR;
		++tcycles;
		icycles = tcycles;
		if (tcycles - icycles >= 1 || icycles - tcycles >= 1)
			return NULL;
		seconds = icycles;
		seconds *= YEARSPERREPEAT;
		seconds *= AVGSECSPERYEAR;
		if (t < sp->ats[0])
			newt += seconds;
		else
			newt -= seconds;
		if (newt < sp->ats[0] ||
			newt > sp->ats[sp->timecnt - 1])
			return NULL;		/* "cannot happen" */
		result = localsub(&newt, offset, tmp, tz);
		if (result == tmp)
		{
			pg_time_t	newy;

			newy = tmp->tm_year;
			if (t < sp->ats[0])
				newy -= icycles * YEARSPERREPEAT;
			else
				newy += icycles * YEARSPERREPEAT;
			tmp->tm_year = newy;
			if (tmp->tm_year != newy)
				return NULL;
		}
		return result;
	}
	if (sp->timecnt == 0 || t < sp->ats[0])
	{
		i = 0;
		while (sp->ttis[i].tt_isdst)
			if (++i >= sp->typecnt)
			{
				i = 0;
				break;
			}
	}
	else
	{
		int			lo = 1;
		int			hi = sp->timecnt;

		while (lo < hi)
		{
			int			mid = (lo + hi) >> 1;

			if (t < sp->ats[mid])
				hi = mid;
			else
				lo = mid + 1;
		}
		i = (int) sp->types[lo - 1];
	}
	ttisp = &sp->ttis[i];

	result = timesub(&t, ttisp->tt_gmtoff, sp, tmp);
	tmp->tm_isdst = ttisp->tt_isdst;
	tmp->tm_zone = &sp->chars[ttisp->tt_abbrind];
	return result;
}


