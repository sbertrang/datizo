/* postgresql: src/timezone/localtime.c */

#include <sys/types.h>
#include <unistd.h>

#include "datizo.h"

int
tzload(const char *name, char *canonname, struct state * sp, int doextend)
{
	const char *p;
	int			i;
	int			fid;
	int			stored;
	int			nread;
	union
	{
		struct tzhead tzhead;
		char		buf[2 * sizeof(struct tzhead) +
									2 * sizeof *sp +
									4 * TZ_MAX_TIMES];
	}			u;

	sp->goback = sp->goahead = FALSE;
	if (name == NULL && (name = TZDEFAULT) == NULL)
		return -1;
	if (name[0] == ':')
		++name;
	fid = pg_open_tzfile(name, canonname);
	if (fid < 0)
		return -1;
	nread = read(fid, u.buf, sizeof u.buf);
	if (close(fid) != 0 || nread <= 0)
		return -1;
	for (stored = 4; stored <= 8; stored *= 2)
	{
		int			ttisstdcnt;
		int			ttisgmtcnt;

		ttisstdcnt = (int) detzcode(u.tzhead.tzh_ttisstdcnt);
		ttisgmtcnt = (int) detzcode(u.tzhead.tzh_ttisgmtcnt);
		sp->leapcnt = (int) detzcode(u.tzhead.tzh_leapcnt);
		sp->timecnt = (int) detzcode(u.tzhead.tzh_timecnt);
		sp->typecnt = (int) detzcode(u.tzhead.tzh_typecnt);
		sp->charcnt = (int) detzcode(u.tzhead.tzh_charcnt);
		p = u.tzhead.tzh_charcnt + sizeof u.tzhead.tzh_charcnt;
		if (sp->leapcnt < 0 || sp->leapcnt > TZ_MAX_LEAPS ||
			sp->typecnt <= 0 || sp->typecnt > TZ_MAX_TYPES ||
			sp->timecnt < 0 || sp->timecnt > TZ_MAX_TIMES ||
			sp->charcnt < 0 || sp->charcnt > TZ_MAX_CHARS ||
			(ttisstdcnt != sp->typecnt && ttisstdcnt != 0) ||
			(ttisgmtcnt != sp->typecnt && ttisgmtcnt != 0))
			return -1;
		if (nread - (p - u.buf) <
			sp->timecnt * stored +		/* ats */
			sp->timecnt +		/* types */
			sp->typecnt * 6 +	/* ttinfos */
			sp->charcnt +		/* chars */
			sp->leapcnt * (stored + 4) +		/* lsinfos */
			ttisstdcnt +		/* ttisstds */
			ttisgmtcnt)			/* ttisgmts */
			return -1;
		for (i = 0; i < sp->timecnt; ++i)
		{
			sp->ats[i] = (stored == 4) ? detzcode(p) : detzcode64(p);
			p += stored;
		}
		for (i = 0; i < sp->timecnt; ++i)
		{
			sp->types[i] = (unsigned char) *p++;
			if (sp->types[i] >= sp->typecnt)
				return -1;
		}
		for (i = 0; i < sp->typecnt; ++i)
		{
			struct ttinfo *ttisp;

			ttisp = &sp->ttis[i];
			ttisp->tt_gmtoff = detzcode(p);
			p += 4;
			ttisp->tt_isdst = (unsigned char) *p++;
			if (ttisp->tt_isdst != 0 && ttisp->tt_isdst != 1)
				return -1;
			ttisp->tt_abbrind = (unsigned char) *p++;
			if (ttisp->tt_abbrind < 0 ||
				ttisp->tt_abbrind > sp->charcnt)
				return -1;
		}
		for (i = 0; i < sp->charcnt; ++i)
			sp->chars[i] = *p++;
		sp->chars[i] = '\0';	/* ensure '\0' at end */
		for (i = 0; i < sp->leapcnt; ++i)
		{
			struct lsinfo *lsisp;

			lsisp = &sp->lsis[i];
			lsisp->ls_trans = (stored == 4) ? detzcode(p) : detzcode64(p);
			p += stored;
			lsisp->ls_corr = detzcode(p);
			p += 4;
		}
		for (i = 0; i < sp->typecnt; ++i)
		{
			struct ttinfo *ttisp;

			ttisp = &sp->ttis[i];
			if (ttisstdcnt == 0)
				ttisp->tt_ttisstd = FALSE;
			else
			{
				ttisp->tt_ttisstd = *p++;
				if (ttisp->tt_ttisstd != TRUE &&
					ttisp->tt_ttisstd != FALSE)
					return -1;
			}
		}
		for (i = 0; i < sp->typecnt; ++i)
		{
			struct ttinfo *ttisp;

			ttisp = &sp->ttis[i];
			if (ttisgmtcnt == 0)
				ttisp->tt_ttisgmt = FALSE;
			else
			{
				ttisp->tt_ttisgmt = *p++;
				if (ttisp->tt_ttisgmt != TRUE &&
					ttisp->tt_ttisgmt != FALSE)
					return -1;
			}
		}

		/*
		 * Out-of-sort ats should mean we're running on a signed time_t system
		 * but using a data file with unsigned values (or vice versa).
		 */
		for (i = 0; i < sp->timecnt - 2; ++i)
			if (sp->ats[i] > sp->ats[i + 1])
			{
				++i;
				if (TYPE_SIGNED(pg_time_t))
				{
					/*
					 * Ignore the end (easy).
					 */
					sp->timecnt = i;
				}
				else
				{
					/*
					 * Ignore the beginning (harder).
					 */
					int			j;

					for (j = 0; j + i < sp->timecnt; ++j)
					{
						sp->ats[j] = sp->ats[j + i];
						sp->types[j] = sp->types[j + i];
					}
					sp->timecnt = j;
				}
				break;
			}

		/*
		 * If this is an old file, we're done.
		 */
		if (u.tzhead.tzh_version[0] == '\0')
			break;
		nread -= p - u.buf;
		for (i = 0; i < nread; ++i)
			u.buf[i] = p[i];

		/*
		 * If this is a narrow integer time_t system, we're done.
		 */
		if (stored >= (int) sizeof(pg_time_t) && TYPE_INTEGRAL(pg_time_t))
			break;
	}
	if (doextend && nread > 2 &&
		u.buf[0] == '\n' && u.buf[nread - 1] == '\n' &&
		sp->typecnt + 2 <= TZ_MAX_TYPES)
	{
		struct state ts;
		int			result;

		u.buf[nread - 1] = '\0';
		result = tzparse(&u.buf[1], &ts, FALSE);
		if (result == 0 && ts.typecnt == 2 &&
			sp->charcnt + ts.charcnt <= TZ_MAX_CHARS)
		{
			for (i = 0; i < 2; ++i)
				ts.ttis[i].tt_abbrind +=
					sp->charcnt;
			for (i = 0; i < ts.charcnt; ++i)
				sp->chars[sp->charcnt++] =
					ts.chars[i];
			i = 0;
			while (i < ts.timecnt &&
				   ts.ats[i] <=
				   sp->ats[sp->timecnt - 1])
				++i;
			while (i < ts.timecnt &&
				   sp->timecnt < TZ_MAX_TIMES)
			{
				sp->ats[sp->timecnt] =
					ts.ats[i];
				sp->types[sp->timecnt] =
					sp->typecnt +
					ts.types[i];
				++sp->timecnt;
				++i;
			}
			sp->ttis[sp->typecnt++] = ts.ttis[0];
			sp->ttis[sp->typecnt++] = ts.ttis[1];
		}
	}
	if (sp->timecnt > 1)
	{
		for (i = 1; i < sp->timecnt; ++i)
			if (typesequiv(sp, sp->types[i], sp->types[0]) &&
				differ_by_repeat(sp->ats[i], sp->ats[0]))
			{
				sp->goback = TRUE;
				break;
			}
		for (i = sp->timecnt - 2; i >= 0; --i)
			if (typesequiv(sp, sp->types[sp->timecnt - 1],
						   sp->types[i]) &&
				differ_by_repeat(sp->ats[sp->timecnt - 1],
								 sp->ats[i]))
			{
				sp->goahead = TRUE;
				break;
			}
	}
	return 0;
}


