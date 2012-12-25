/* postgresql: src/timezone/localtime.c */

#include "datizo.h"

/*
 * Given the Epoch-relative time of January 1, 00:00:00 UTC, in a year, the
 * year, a rule, and the offset from UTC at the time that rule takes effect,
 * calculate the Epoch-relative time that rule takes effect.
 */
pg_time_t
transtime(pg_time_t janfirst, int year,
		  const struct rule * rulep, long offset)
{
	int			leapyear;
	pg_time_t	value = 0;
	int			i,
				d,
				m1,
				yy0,
				yy1,
				yy2,
				dow;

	leapyear = isleap(year);
	switch (rulep->r_type)
	{

		case JULIAN_DAY:

			/*
			 * Jn - Julian day, 1 == January 1, 60 == March 1 even in leap
			 * years. In non-leap years, or if the day number is 59 or less,
			 * just add SECSPERDAY times the day number-1 to the time of
			 * January 1, midnight, to get the day.
			 */
			value = janfirst + (rulep->r_day - 1) * SECSPERDAY;
			if (leapyear && rulep->r_day >= 60)
				value += SECSPERDAY;
			break;

		case DAY_OF_YEAR:

			/*
			 * n - day of year. Just add SECSPERDAY times the day number to
			 * the time of January 1, midnight, to get the day.
			 */
			value = janfirst + rulep->r_day * SECSPERDAY;
			break;

		case MONTH_NTH_DAY_OF_WEEK:

			/*
			 * Mm.n.d - nth "dth day" of month m.
			 */
			value = janfirst;
			for (i = 0; i < rulep->r_mon - 1; ++i)
				value += mon_lengths[leapyear][i] * SECSPERDAY;

			/*
			 * Use Zeller's Congruence to get day-of-week of first day of
			 * month.
			 */
			m1 = (rulep->r_mon + 9) % 12 + 1;
			yy0 = (rulep->r_mon <= 2) ? (year - 1) : year;
			yy1 = yy0 / 100;
			yy2 = yy0 % 100;
			dow = ((26 * m1 - 2) / 10 +
				   1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
			if (dow < 0)
				dow += DAYSPERWEEK;

			/*
			 * "dow" is the day-of-week of the first day of the month. Get the
			 * day-of-month (zero-origin) of the first "dow" day of the month.
			 */
			d = rulep->r_day - dow;
			if (d < 0)
				d += DAYSPERWEEK;
			for (i = 1; i < rulep->r_week; ++i)
			{
				if (d + DAYSPERWEEK >=
					mon_lengths[leapyear][rulep->r_mon - 1])
					break;
				d += DAYSPERWEEK;
			}

			/*
			 * "d" is the day-of-month (zero-origin) of the day we want.
			 */
			value += d * SECSPERDAY;
			break;
	}

	/*
	 * "value" is the Epoch-relative time of 00:00:00 UTC on the day in
	 * question.  To get the Epoch-relative time of the specified local time
	 * on that day, add the transition time and the current offset from UTC.
	 */
	return value + rulep->r_time + offset;
}

