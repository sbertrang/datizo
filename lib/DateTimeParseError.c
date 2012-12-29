/* postgresql: src/backend/utils/adt/datetime.c */

#include "datizo.h"

/*
 * Report an error detected by one of the datetime input processing routines.
 *
 * dterr is the error code, str is the original input string, datatype is
 * the name of the datatype we were trying to accept.
 *
 * Note: it might seem useless to distinguish DTERR_INTERVAL_OVERFLOW and
 * DTERR_TZDISP_OVERFLOW from DTERR_FIELD_OVERFLOW, but SQL99 mandates three
 * separate SQLSTATE codes, so ...
 */
void
DateTimeParseError(int dterr, const char *str, const char *datatype)
{
	switch (dterr)
	{
		case DTERR_FIELD_OVERFLOW:
			warnx("date/time field value out of range: \"%s\"",
							str);
			break;
		case DTERR_MD_FIELD_OVERFLOW:
			/* <nanny>same as above, but add hint about DateStyle</nanny> */
			warnx("date/time field value out of range: \"%s\"" " (" "Perhaps you need a different \"datestyle\" setting." ")" ,
							str);
			break;
		case DTERR_INTERVAL_OVERFLOW:
			warnx("interval field value out of range: \"%s\"",
							str);
			break;
		case DTERR_TZDISP_OVERFLOW:
			warnx("time zone displacement out of range: \"%s\"",
							str);
			break;
		case DTERR_BAD_FORMAT:
		default:
			warnx("invalid input syntax for type %s: \"%s\"",
							datatype, str);
			break;
	}
}

