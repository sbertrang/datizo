
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "datizo.h"


/* EncodeDateTime()
 * Encode date and time interpreted as local time.
 *
 * tm and fsec are the value to encode, print_tz determines whether to include
 * a time zone (the difference between timestamp and timestamptz types), tz is
 * the numeric time zone offset, tzn is the textual time zone, which if
 * specified will be used instead of tz by some styles, style is the date
 * style, str is where to write the output.
 *
 * Supported date styles:
 *	Postgres - day mon hh:mm:ss yyyy tz
 *	SQL - mm/dd/yyyy hh:mm:ss.ss tz
 *	ISO - yyyy-mm-dd hh:mm:ss+/-tz
 *	German - dd.mm.yyyy hh:mm:ss tz
 *	XSD - yyyy-mm-ddThh:mm:ss.ss+/-tz
 */
void
EncodeDateTime(struct tm * tm, fsec_t fsec, bool print_tz, int tz, const char *tzn, int style, char *str)
{
	int			day;

	assert(tm->tm_mon >= 1 && tm->tm_mon <= MONTHS_PER_YEAR);

	/*
	 * Negative tm_isdst means we have no valid time zone translation.
	 */
	if (tm->tm_isdst < 0)
		print_tz = false;

	switch (style)
	{
		case USE_ISO_DATES:
		case USE_XSD_DATES:
			/* Compatible with ISO-8601 date formats */

			if (style == USE_ISO_DATES)
				sprintf(str, "%04d-%02d-%02d %02d:%02d:",
						(tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1),
						tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min);
			else
				sprintf(str, "%04d-%02d-%02dT%02d:%02d:",
						(tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1),
						tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min);

			AppendTimestampSeconds(str + strlen(str), tm, fsec);

			if (print_tz)
				EncodeTimezone(str, tz, style);

			if (tm->tm_year <= 0)
				sprintf(str + strlen(str), " BC");
			break;

		case USE_SQL_DATES:
			/* Compatible with Oracle/Ingres date formats */

			if (DateOrder == DATEORDER_DMY)
				sprintf(str, "%02d/%02d", tm->tm_mday, tm->tm_mon);
			else
				sprintf(str, "%02d/%02d", tm->tm_mon, tm->tm_mday);

			sprintf(str + 5, "/%04d %02d:%02d:",
					(tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1),
					tm->tm_hour, tm->tm_min);

			AppendTimestampSeconds(str + strlen(str), tm, fsec);

			/*
			 * Note: the uses of %.*s in this function would be risky if the
			 * timezone names ever contain non-ASCII characters.  However, all
			 * TZ abbreviations in the Olson database are plain ASCII.
			 */

			if (print_tz)
			{
				if (tzn)
					sprintf(str + strlen(str), " %.*s", MAXTZLEN, tzn);
				else
					EncodeTimezone(str, tz, style);
			}

			if (tm->tm_year <= 0)
				sprintf(str + strlen(str), " BC");
			break;

		case USE_GERMAN_DATES:
			/* German variant on European style */

			sprintf(str, "%02d.%02d", tm->tm_mday, tm->tm_mon);

			sprintf(str + 5, ".%04d %02d:%02d:",
					(tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1),
					tm->tm_hour, tm->tm_min);

			AppendTimestampSeconds(str + strlen(str), tm, fsec);

			if (print_tz)
			{
				if (tzn)
					sprintf(str + strlen(str), " %.*s", MAXTZLEN, tzn);
				else
					EncodeTimezone(str, tz, style);
			}

			if (tm->tm_year <= 0)
				sprintf(str + strlen(str), " BC");
			break;

		case USE_POSTGRES_DATES:
		default:
			/* Backward-compatible with traditional Postgres abstime dates */

			day = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday);
			tm->tm_wday = j2day(day);

			strncpy(str, days[tm->tm_wday], 3);
			strcpy(str + 3, " ");

			if (DateOrder == DATEORDER_DMY)
				sprintf(str + 4, "%02d %3s", tm->tm_mday, months[tm->tm_mon - 1]);
			else
				sprintf(str + 4, "%3s %02d", months[tm->tm_mon - 1], tm->tm_mday);

			sprintf(str + 10, " %02d:%02d:", tm->tm_hour, tm->tm_min);

			AppendTimestampSeconds(str + strlen(str), tm, fsec);

			sprintf(str + strlen(str), " %04d",
					(tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1));

			if (print_tz)
			{
				if (tzn)
					sprintf(str + strlen(str), " %.*s", MAXTZLEN, tzn);
				else
				{
					/*
					 * We have a time zone, but no string version. Use the
					 * numeric form, but be sure to include a leading space to
					 * avoid formatting something which would be rejected by
					 * the date/time parser later. - thomas 2001-10-19
					 */
					sprintf(str + strlen(str), " ");
					EncodeTimezone(str, tz, style);
				}
			}

			if (tm->tm_year <= 0)
				sprintf(str + strlen(str), " BC");
			break;
	}
}

