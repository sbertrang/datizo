
#include <string.h>

#include "datizo.h"

/* DecodeISO8601Interval()
 *	Decode an ISO 8601 time interval of the "format with designators"
 *	(section 4.4.3.2) or "alternative format" (section 4.4.3.3)
 *	Examples:  P1D	for 1 day
 *			   PT1H for 1 hour
 *			   P2Y6M7DT1H30M for 2 years, 6 months, 7 days 1 hour 30 min
 *			   P0002-06-07T01:30:00 the same value in alternative format
 *
 * Returns 0 if successful, DTERR code if bogus input detected.
 * Note: error code should be DTERR_BAD_FORMAT if input doesn't look like
 * ISO8601, otherwise this could cause unexpected error messages.
 * dtype, tm, fsec are output parameters.
 *
 *	A couple exceptions from the spec:
 *	 - a week field ('W') may coexist with other units
 *	 - allows decimals in fields other than the least significant unit.
 */
int
DecodeISO8601Interval(char *str,
					  int *dtype, struct tm * tm, fsec_t *fsec)
{
	bool		datepart = true;
	bool		havefield = false;

	*dtype = DTK_DELTA;
	ClearPgTm(tm, fsec);

	if (strlen(str) < 2 || str[0] != 'P')
		return DTERR_BAD_FORMAT;

	str++;
	while (*str)
	{
		char	   *fieldstart;
		int			val;
		double		fval;
		char		unit;
		int			dterr;

		if (*str == 'T')		/* T indicates the beginning of the time part */
		{
			datepart = false;
			havefield = false;
			str++;
			continue;
		}

		fieldstart = str;
		dterr = ParseISO8601Number(str, &str, &val, &fval);
		if (dterr)
			return dterr;

		/*
		 * Note: we could step off the end of the string here.	Code below
		 * *must* exit the loop if unit == '\0'.
		 */
		unit = *str++;

		if (datepart)
		{
			switch (unit)		/* before T: Y M W D */
			{
				case 'Y':
					tm->tm_year += val;
					tm->tm_mon += (fval * MONTHS_PER_YEAR);
					break;
				case 'M':
					tm->tm_mon += val;
					AdjustFractDays(fval, tm, fsec, DAYS_PER_MONTH);
					break;
				case 'W':
					tm->tm_mday += val * 7;
					AdjustFractDays(fval, tm, fsec, 7);
					break;
				case 'D':
					tm->tm_mday += val;
					AdjustFractSeconds(fval, tm, fsec, SECS_PER_DAY);
					break;
				case 'T':		/* ISO 8601 4.4.3.3 Alternative Format / Basic */
				case '\0':
					if (ISO8601IntegerWidth(fieldstart) == 8 && !havefield)
					{
						tm->tm_year += val / 10000;
						tm->tm_mon += (val / 100) % 100;
						tm->tm_mday += val % 100;
						AdjustFractSeconds(fval, tm, fsec, SECS_PER_DAY);
						if (unit == '\0')
							return 0;
						datepart = false;
						havefield = false;
						continue;
					}
					/* Else fall through to extended alternative format */
				case '-':		/* ISO 8601 4.4.3.3 Alternative Format,
								 * Extended */
					if (havefield)
						return DTERR_BAD_FORMAT;

					tm->tm_year += val;
					tm->tm_mon += (fval * MONTHS_PER_YEAR);
					if (unit == '\0')
						return 0;
					if (unit == 'T')
					{
						datepart = false;
						havefield = false;
						continue;
					}

					dterr = ParseISO8601Number(str, &str, &val, &fval);
					if (dterr)
						return dterr;
					tm->tm_mon += val;
					AdjustFractDays(fval, tm, fsec, DAYS_PER_MONTH);
					if (*str == '\0')
						return 0;
					if (*str == 'T')
					{
						datepart = false;
						havefield = false;
						continue;
					}
					if (*str != '-')
						return DTERR_BAD_FORMAT;
					str++;

					dterr = ParseISO8601Number(str, &str, &val, &fval);
					if (dterr)
						return dterr;
					tm->tm_mday += val;
					AdjustFractSeconds(fval, tm, fsec, SECS_PER_DAY);
					if (*str == '\0')
						return 0;
					if (*str == 'T')
					{
						datepart = false;
						havefield = false;
						continue;
					}
					return DTERR_BAD_FORMAT;
				default:
					/* not a valid date unit suffix */
					return DTERR_BAD_FORMAT;
			}
		}
		else
		{
			switch (unit)		/* after T: H M S */
			{
				case 'H':
					tm->tm_hour += val;
					AdjustFractSeconds(fval, tm, fsec, SECS_PER_HOUR);
					break;
				case 'M':
					tm->tm_min += val;
					AdjustFractSeconds(fval, tm, fsec, SECS_PER_MINUTE);
					break;
				case 'S':
					tm->tm_sec += val;
					AdjustFractSeconds(fval, tm, fsec, 1);
					break;
				case '\0':		/* ISO 8601 4.4.3.3 Alternative Format */
					if (ISO8601IntegerWidth(fieldstart) == 6 && !havefield)
					{
						tm->tm_hour += val / 10000;
						tm->tm_min += (val / 100) % 100;
						tm->tm_sec += val % 100;
						AdjustFractSeconds(fval, tm, fsec, 1);
						return 0;
					}
					/* Else fall through to extended alternative format */
				case ':':		/* ISO 8601 4.4.3.3 Alternative Format,
								 * Extended */
					if (havefield)
						return DTERR_BAD_FORMAT;

					tm->tm_hour += val;
					AdjustFractSeconds(fval, tm, fsec, SECS_PER_HOUR);
					if (unit == '\0')
						return 0;

					dterr = ParseISO8601Number(str, &str, &val, &fval);
					if (dterr)
						return dterr;
					tm->tm_min += val;
					AdjustFractSeconds(fval, tm, fsec, SECS_PER_MINUTE);
					if (*str == '\0')
						return 0;
					if (*str != ':')
						return DTERR_BAD_FORMAT;
					str++;

					dterr = ParseISO8601Number(str, &str, &val, &fval);
					if (dterr)
						return dterr;
					tm->tm_sec += val;
					AdjustFractSeconds(fval, tm, fsec, 1);
					if (*str == '\0')
						return 0;
					return DTERR_BAD_FORMAT;

				default:
					/* not a valid time unit suffix */
					return DTERR_BAD_FORMAT;
			}
		}

		havefield = true;
	}

	return 0;
}


