
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datizo.h"


/* EncodeInterval()
 * Interpret time structure as a delta time and convert to string.
 *
 * Support "traditional Postgres" and ISO-8601 styles.
 * Actually, afaik ISO does not address time interval formatting,
 *	but this looks similar to the spec for absolute date/time.
 * - thomas 1998-04-30
 *
 * Actually, afaik, ISO 8601 does specify formats for "time
 * intervals...[of the]...format with time-unit designators", which
 * are pretty ugly.  The format looks something like
 *	   P1Y1M1DT1H1M1.12345S
 * but useful for exchanging data with computers instead of humans.
 * - ron 2003-07-14
 *
 * And ISO's SQL 2008 standard specifies standards for
 * "year-month literal"s (that look like '2-3') and
 * "day-time literal"s (that look like ('4 5:6:7')
 */
void
EncodeInterval(struct tm * tm, fsec_t fsec, int style, char *str)
{
	char	   *cp = str;
	int			year = tm->tm_year;
	int			mon = tm->tm_mon;
	int			mday = tm->tm_mday;
	int			hour = tm->tm_hour;
	int			min = tm->tm_min;
	int			sec = tm->tm_sec;
	bool		is_before = FALSE;
	bool		is_zero = TRUE;

	/*
	 * The sign of year and month are guaranteed to match, since they are
	 * stored internally as "month". But we'll need to check for is_before and
	 * is_zero when determining the signs of day and hour/minute/seconds
	 * fields.
	 */
	switch (style)
	{
			/* SQL Standard interval format */
		case INTSTYLE_SQL_STANDARD:
			{
				bool		has_negative = year < 0 || mon < 0 ||
				mday < 0 || hour < 0 ||
				min < 0 || sec < 0 || fsec < 0;
				bool		has_positive = year > 0 || mon > 0 ||
				mday > 0 || hour > 0 ||
				min > 0 || sec > 0 || fsec > 0;
				bool		has_year_month = year != 0 || mon != 0;
				bool		has_day_time = mday != 0 || hour != 0 ||
				min != 0 || sec != 0 || fsec != 0;
				bool		has_day = mday != 0;
				bool		sql_standard_value = !(has_negative && has_positive) &&
				!(has_year_month && has_day_time);

				/*
				 * SQL Standard wants only 1 "<sign>" preceding the whole
				 * interval ... but can't do that if mixed signs.
				 */
				if (has_negative && sql_standard_value)
				{
					*cp++ = '-';
					year = -year;
					mon = -mon;
					mday = -mday;
					hour = -hour;
					min = -min;
					sec = -sec;
					fsec = -fsec;
				}

				if (!has_negative && !has_positive)
				{
					sprintf(cp, "0");
				}
				else if (!sql_standard_value)
				{
					/*
					 * For non sql-standard interval values, force outputting
					 * the signs to avoid ambiguities with intervals with
					 * mixed sign components.
					 */
					char		year_sign = (year < 0 || mon < 0) ? '-' : '+';
					char		day_sign = (mday < 0) ? '-' : '+';
					char		sec_sign = (hour < 0 || min < 0 ||
											sec < 0 || fsec < 0) ? '-' : '+';

					sprintf(cp, "%c%d-%d %c%d %c%d:%02d:",
							year_sign, abs(year), abs(mon),
							day_sign, abs(mday),
							sec_sign, abs(hour), abs(min));
					cp += strlen(cp);
					AppendSeconds(cp, sec, fsec, MAX_INTERVAL_PRECISION, true);
				}
				else if (has_year_month)
				{
					sprintf(cp, "%d-%d", year, mon);
				}
				else if (has_day)
				{
					sprintf(cp, "%d %d:%02d:", mday, hour, min);
					cp += strlen(cp);
					AppendSeconds(cp, sec, fsec, MAX_INTERVAL_PRECISION, true);
				}
				else
				{
					sprintf(cp, "%d:%02d:", hour, min);
					cp += strlen(cp);
					AppendSeconds(cp, sec, fsec, MAX_INTERVAL_PRECISION, true);
				}
			}
			break;

			/* ISO 8601 "time-intervals by duration only" */
		case INTSTYLE_ISO_8601:
			/* special-case zero to avoid printing nothing */
			if (year == 0 && mon == 0 && mday == 0 &&
				hour == 0 && min == 0 && sec == 0 && fsec == 0)
			{
				sprintf(cp, "PT0S");
				break;
			}
			*cp++ = 'P';
			cp = AddISO8601IntPart(cp, year, 'Y');
			cp = AddISO8601IntPart(cp, mon, 'M');
			cp = AddISO8601IntPart(cp, mday, 'D');
			if (hour != 0 || min != 0 || sec != 0 || fsec != 0)
				*cp++ = 'T';
			cp = AddISO8601IntPart(cp, hour, 'H');
			cp = AddISO8601IntPart(cp, min, 'M');
			if (sec != 0 || fsec != 0)
			{
				if (sec < 0 || fsec < 0)
					*cp++ = '-';
				AppendSeconds(cp, sec, fsec, MAX_INTERVAL_PRECISION, false);
				cp += strlen(cp);
				*cp++ = 'S';
				*cp++ = '\0';
			}
			break;

			/* Compatible with postgresql < 8.4 when DateStyle = 'iso' */
		case INTSTYLE_POSTGRES:
			cp = AddPostgresIntPart(cp, year, "year", &is_zero, &is_before);

			/*
			 * Ideally we should spell out "month" like we do for "year" and
			 * "day".  However, for backward compatibility, we can't easily
			 * fix this.  bjm 2011-05-24
			 */
			cp = AddPostgresIntPart(cp, mon, "mon", &is_zero, &is_before);
			cp = AddPostgresIntPart(cp, mday, "day", &is_zero, &is_before);
			if (is_zero || hour != 0 || min != 0 || sec != 0 || fsec != 0)
			{
				bool		minus = (hour < 0 || min < 0 || sec < 0 || fsec < 0);

				sprintf(cp, "%s%s%02d:%02d:",
						is_zero ? "" : " ",
						(minus ? "-" : (is_before ? "+" : "")),
						abs(hour), abs(min));
				cp += strlen(cp);
				AppendSeconds(cp, sec, fsec, MAX_INTERVAL_PRECISION, true);
			}
			break;

			/* Compatible with postgresql < 8.4 when DateStyle != 'iso' */
		case INTSTYLE_POSTGRES_VERBOSE:
		default:
			strcpy(cp, "@");
			cp++;
			cp = AddVerboseIntPart(cp, year, "year", &is_zero, &is_before);
			cp = AddVerboseIntPart(cp, mon, "mon", &is_zero, &is_before);
			cp = AddVerboseIntPart(cp, mday, "day", &is_zero, &is_before);
			cp = AddVerboseIntPart(cp, hour, "hour", &is_zero, &is_before);
			cp = AddVerboseIntPart(cp, min, "min", &is_zero, &is_before);
			if (sec != 0 || fsec != 0)
			{
				*cp++ = ' ';
				if (sec < 0 || (sec == 0 && fsec < 0))
				{
					if (is_zero)
						is_before = TRUE;
					else if (!is_before)
						*cp++ = '-';
				}
				else if (is_before)
					*cp++ = '-';
				AppendSeconds(cp, sec, fsec, MAX_INTERVAL_PRECISION, false);
				cp += strlen(cp);
				sprintf(cp, " sec%s",
						(abs(sec) != 1 || fsec != 0) ? "s" : "");
				is_zero = FALSE;
			}
			/* identically zero? then put in a unitless zero... */
			if (is_zero)
				strcat(cp, " 0");
			if (is_before)
				strcat(cp, " ago");
			break;
	}
}


