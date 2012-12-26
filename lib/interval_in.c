/* postgresql: src/backend/utils/adt/timestamp.c */

#include <stdlib.h>

#include "datizo.h"

/* interval_in()
 * Convert a string to internal form.
 *
 * External format(s):
 *	Uses the generic date/time parsing and decoding routines.
 */
Interval *
interval_in(char *str)
{
	/* char	   *str = PG_GETARG_CSTRING(0); */
	int32_t		typmod = INTERVAL_TYPMOD(MAX_INTERVAL_PRECISION, INTERVAL_FULL_RANGE); /* PG_GETARG_INT32(2); */
	Interval   *result;
	fsec_t		fsec;
	struct tm tt,
			   *tm = &tt;
	int			dtype;
	int			nf;
	int			range;
	int			dterr;
	char	   *field[MAXDATEFIELDS];
	int			ftype[MAXDATEFIELDS];
	char		workbuf[256];

	tm->tm_year = 0;
	tm->tm_mon = 0;
	tm->tm_mday = 0;
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	fsec = 0;

	if (typmod >= 0)
		range = INTERVAL_RANGE(typmod);
	else
		range = INTERVAL_FULL_RANGE;

	dterr = ParseDateTime(str, workbuf, sizeof(workbuf), field,
						  ftype, MAXDATEFIELDS, &nf);
	if (dterr == 0)
		dterr = DecodeInterval(field, ftype, nf, range,
							   &dtype, tm, &fsec);

	/* if those functions think it's a bad format, try ISO8601 style */
	if (dterr == DTERR_BAD_FORMAT)
		dterr = DecodeISO8601Interval(str,
									  &dtype, tm, &fsec);

	if (dterr != 0)
	{
		if (dterr == DTERR_FIELD_OVERFLOW)
			dterr = DTERR_INTERVAL_OVERFLOW;
		DateTimeParseError(dterr, str, "interval");
	}

	result = (Interval *) malloc(sizeof(Interval));

	switch (dtype)
	{
		case DTK_DELTA:
			if (tm2interval(tm, fsec, result) != 0)
				warnx("interval out of range");
			break;

		case DTK_INVALID:
			warnx("date/time value \"%s\" is no longer supported", str);
			break;

		default:
			warnx("unexpected dtype %d while parsing interval \"%s\"",
				 dtype, str);
	}

	AdjustIntervalForTypmod(result, typmod);

	return result;
}


