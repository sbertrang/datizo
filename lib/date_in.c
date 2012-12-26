/* postgresql: src/backend/utils/adt/date.c */

#include "datizo.h"

/* date_in()
 * Given date text string, convert to internal date format.
 */
DateADT
date_in(char *str)
{
	/*char	   *str = PG_GETARG_CSTRING(0);*/
	DateADT		date;
	fsec_t		fsec;
	struct tm tt,
			   *tm = &tt;
	int			tzp;
	int			dtype;
	int			nf;
	int			dterr;
	char	   *field[MAXDATEFIELDS];
	int			ftype[MAXDATEFIELDS];
	char		workbuf[MAXDATELEN + 1];

	dterr = ParseDateTime(str, workbuf, sizeof(workbuf),
						  field, ftype, MAXDATEFIELDS, &nf);
	if (dterr == 0)
		dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tzp);
	if (dterr != 0)
		DateTimeParseError(dterr, str, "date");

	switch (dtype)
	{
		case DTK_DATE:
			break;

		case DTK_CURRENT:
			warnx("date/time value \"current\" is no longer supported");

			GetCurrentDateTime(tm);
			break;

		case DTK_EPOCH:
			GetEpochTime(tm);
			break;

		case DTK_LATE:
			DATE_NOEND(date);
			return date;

		case DTK_EARLY:
			DATE_NOBEGIN(date);
			return date;

		default:
			DateTimeParseError(DTERR_BAD_FORMAT, str, "date");
			break;
	}

	if (!IS_VALID_JULIAN(tm->tm_year, tm->tm_mon, tm->tm_mday))
		warnx("date out of range: \"%s\"", str);

	date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;

	return date;
}

