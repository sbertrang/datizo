/* postgresql: src/backend/utils/adt/timestamp.c */

/*******************************************************************************
********************************************************************************
**  src/backend/utils/adt/timestamp.c  *****************************************
********************************************************************************
*******************************************************************************/

#include "datizo.h"

/* timestamptz_in()
 * Convert a string to internal form.
 */
TimestampTz
timestamptz_in(char *str)
{
	/* char	   *str = PG_GETARG_CSTRING(0); */
	int32_t		typmod = 0; /* PG_GETARG_INT32(2); */
	TimestampTz result;
	fsec_t		fsec;
	struct tm tt,
			   *tm = &tt;
	int			tz;
	int			dtype;
	int			nf;
	int			dterr;
	char	   *field[MAXDATEFIELDS];
	int			ftype[MAXDATEFIELDS];
	char		workbuf[MAXDATELEN + MAXDATEFIELDS];

	dterr = ParseDateTime(str, workbuf, sizeof(workbuf),
						  field, ftype, MAXDATEFIELDS, &nf);
	if (dterr == 0)
		dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tz);
	if (dterr != 0)
		DateTimeParseError(dterr, str, "timestamp with time zone");

	switch (dtype)
	{
		case DTK_DATE:
			if (tm2timestamp(tm, fsec, &tz, &result) != 0)
				warnx("timestamp out of range: \"%s\"", str);
			break;

		case DTK_EPOCH:
			result = SetEpochTimestamp();
			break;

		case DTK_LATE:
			TIMESTAMP_NOEND(result);
			break;

		case DTK_EARLY:
			TIMESTAMP_NOBEGIN(result);
			break;

		case DTK_INVALID:
			warnx("date/time value \"%s\" is no longer supported", str);

			TIMESTAMP_NOEND(result);
			break;

		default:
			warnx("unexpected dtype %d while parsing timestamptz \"%s\"",
				 dtype, str);
			TIMESTAMP_NOEND(result);
	}

	AdjustTimestampForTypmod(&result, typmod);

	return result;
}

