/* postgresql: src/timezone/localtime.c */

#include <string.h>

#include "datizo.h"

int
tzparse(const char *name, struct state * sp, int lastditch)
{
	const char *stdname;
	const char *dstname = NULL;
	size_t		stdlen;
	size_t		dstlen;
	long		stdoffset;
	long		dstoffset;
	pg_time_t  *atp;
	unsigned char *typep;
	char	   *cp;
	int			load_result;

	stdname = name;
	if (lastditch)
	{
		stdlen = strlen(name);	/* length of standard zone name */
		name += stdlen;
		if (stdlen >= sizeof sp->chars)
			stdlen = (sizeof sp->chars) - 1;
		stdoffset = 0;

		/*
		 * Unlike the original zic library, do NOT invoke tzload() here; we
		 * can't assume pg_open_tzfile() is sane yet, and we don't care about
		 * leap seconds anyway.
		 */
		sp->goback = sp->goahead = FALSE;
		load_result = -1;
	}
	else
	{
		if (*name == '<')
		{
			name++;
			stdname = name;
			name = getqzname(name, '>');
			if (*name != '>')
				return (-1);
			stdlen = name - stdname;
			name++;
		}
		else
		{
			name = getzname(name);
			stdlen = name - stdname;
		}
		if (*name == '\0')
			return -1;
		name = getoffset(name, &stdoffset);
		if (name == NULL)
			return -1;
		load_result = tzload(TZDEFRULES, NULL, sp, FALSE);
	}
	if (load_result != 0)
		sp->leapcnt = 0;		/* so, we're off a little */
	if (*name != '\0')
	{
		if (*name == '<')
		{
			dstname = ++name;
			name = getqzname(name, '>');
			if (*name != '>')
				return -1;
			dstlen = name - dstname;
			name++;
		}
		else
		{
			dstname = name;
			name = getzname(name);
			dstlen = name - dstname;	/* length of DST zone name */
		}
		if (*name != '\0' && *name != ',' && *name != ';')
		{
			name = getoffset(name, &dstoffset);
			if (name == NULL)
				return -1;
		}
		else
			dstoffset = stdoffset - SECSPERHOUR;
		if (*name == '\0' && load_result != 0)
			name = TZDEFRULESTRING;
		if (*name == ',' || *name == ';')
		{
			struct rule start;
			struct rule end;
			int			year;
			pg_time_t	janfirst;
			pg_time_t	starttime;
			pg_time_t	endtime;

			++name;
			if ((name = getrule(name, &start)) == NULL)
				return -1;
			if (*name++ != ',')
				return -1;
			if ((name = getrule(name, &end)) == NULL)
				return -1;
			if (*name != '\0')
				return -1;
			sp->typecnt = 2;	/* standard time and DST */

			/*
			 * Two transitions per year, from EPOCH_YEAR forward.
			 */
			sp->ttis[0].tt_gmtoff = -dstoffset;
			sp->ttis[0].tt_isdst = 1;
			sp->ttis[0].tt_abbrind = stdlen + 1;
			sp->ttis[1].tt_gmtoff = -stdoffset;
			sp->ttis[1].tt_isdst = 0;
			sp->ttis[1].tt_abbrind = 0;
			atp = sp->ats;
			typep = sp->types;
			janfirst = 0;
			sp->timecnt = 0;
			for (year = EPOCH_YEAR;
				 sp->timecnt + 2 <= TZ_MAX_TIMES;
				 ++year)
			{
				pg_time_t	newfirst;

				starttime = transtime(janfirst, year, &start,
									  stdoffset);
				endtime = transtime(janfirst, year, &end,
									dstoffset);
				if (starttime > endtime)
				{
					*atp++ = endtime;
					*typep++ = 1;		/* DST ends */
					*atp++ = starttime;
					*typep++ = 0;		/* DST begins */
				}
				else
				{
					*atp++ = starttime;
					*typep++ = 0;		/* DST begins */
					*atp++ = endtime;
					*typep++ = 1;		/* DST ends */
				}
				sp->timecnt += 2;
				newfirst = janfirst;
				newfirst += year_lengths[isleap(year)] *
					SECSPERDAY;
				if (newfirst <= janfirst)
					break;
				janfirst = newfirst;
			}
		}
		else
		{
			long		theirstdoffset;
			long		theirdstoffset;
			long		theiroffset;
			int			isdst;
			int			i;
			int			j;

			if (*name != '\0')
				return -1;

			/*
			 * Initial values of theirstdoffset and theirdstoffset.
			 */
			theirstdoffset = 0;
			for (i = 0; i < sp->timecnt; ++i)
			{
				j = sp->types[i];
				if (!sp->ttis[j].tt_isdst)
				{
					theirstdoffset =
						-sp->ttis[j].tt_gmtoff;
					break;
				}
			}
			theirdstoffset = 0;
			for (i = 0; i < sp->timecnt; ++i)
			{
				j = sp->types[i];
				if (sp->ttis[j].tt_isdst)
				{
					theirdstoffset =
						-sp->ttis[j].tt_gmtoff;
					break;
				}
			}

			/*
			 * Initially we're assumed to be in standard time.
			 */
			isdst = FALSE;
			theiroffset = theirstdoffset;

			/*
			 * Now juggle transition times and types tracking offsets as you
			 * do.
			 */
			for (i = 0; i < sp->timecnt; ++i)
			{
				j = sp->types[i];
				sp->types[i] = sp->ttis[j].tt_isdst;
				if (sp->ttis[j].tt_ttisgmt)
				{
					/* No adjustment to transition time */
				}
				else
				{
					/*
					 * If summer time is in effect, and the transition time
					 * was not specified as standard time, add the summer time
					 * offset to the transition time; otherwise, add the
					 * standard time offset to the transition time.
					 */

					/*
					 * Transitions from DST to DDST will effectively disappear
					 * since POSIX provides for only one DST offset.
					 */
					if (isdst && !sp->ttis[j].tt_ttisstd)
					{
						sp->ats[i] += dstoffset -
							theirdstoffset;
					}
					else
					{
						sp->ats[i] += stdoffset -
							theirstdoffset;
					}
				}
				theiroffset = -sp->ttis[j].tt_gmtoff;
				if (sp->ttis[j].tt_isdst)
					theirdstoffset = theiroffset;
				else
					theirstdoffset = theiroffset;
			}

			/*
			 * Finally, fill in ttis. ttisstd and ttisgmt need not be handled.
			 */
			sp->ttis[0].tt_gmtoff = -stdoffset;
			sp->ttis[0].tt_isdst = FALSE;
			sp->ttis[0].tt_abbrind = 0;
			sp->ttis[1].tt_gmtoff = -dstoffset;
			sp->ttis[1].tt_isdst = TRUE;
			sp->ttis[1].tt_abbrind = stdlen + 1;
			sp->typecnt = 2;
		}
	}
	else
	{
		dstlen = 0;
		sp->typecnt = 1;		/* only standard time */
		sp->timecnt = 0;
		sp->ttis[0].tt_gmtoff = -stdoffset;
		sp->ttis[0].tt_isdst = 0;
		sp->ttis[0].tt_abbrind = 0;
	}
	sp->charcnt = stdlen + 1;
	if (dstlen != 0)
		sp->charcnt += dstlen + 1;
	if ((size_t) sp->charcnt > sizeof sp->chars)
		return -1;
	cp = sp->chars;
	(void) strncpy(cp, stdname, stdlen);
	cp += stdlen;
	*cp++ = '\0';
	if (dstlen != 0)
	{
		(void) strncpy(cp, dstname, dstlen);
		*(cp + dstlen) = '\0';
	}
	return 0;
}


