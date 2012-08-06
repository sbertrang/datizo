
#include "datizo.h"

TimeADT
time_in(char *str)
{
	/*char	   *str = PG_GETARG_CSTRING(0);*/
	int32_t		typmod = 0; /*PG_GETARG_INT32(2);*/
	TimeADT		result;
	fsec_t		fsec;
	struct tm tt,
			   *tm = &tt;
	int			tz;
	int			nf;
	int			dterr;
	char		workbuf[MAXDATELEN + 1];
	char	   *field[MAXDATEFIELDS];
	int			dtype;
	int			ftype[MAXDATEFIELDS];

	dterr = ParseDateTime(str, workbuf, sizeof(workbuf),
						  field, ftype, MAXDATEFIELDS, &nf);
	if (dterr == 0)
		dterr = DecodeTimeOnly(field, ftype, nf, &dtype, tm, &fsec, &tz);
	if (dterr != 0)
		DateTimeParseError(dterr, str, "time");

	tm2time(tm, fsec, &result);
	AdjustTimeForTypmod(&result, typmod);

	return result;
}

