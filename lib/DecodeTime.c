/* postgresql: src/backend/utils/adt/datetime.c */

#include <errno.h>

#include "datizo.h"

/* DecodeTime()
 * Decode time string which includes delimiters.
 * Return 0 if okay, a DTERR code if not.
 *
 * Only check the lower limit on hours, since this same code can be
 * used to represent time spans.
 */
int
DecodeTime(char *str, int fmask, int range,
		   int *tmask, struct tm * tm, fsec_t *fsec)
{
	char	   *cp;
	int			dterr;

	*tmask = DTK_TIME_M;

	errno = 0;
	tm->tm_hour = strtoi(str, &cp, 10);
	if (errno == ERANGE)
		return DTERR_FIELD_OVERFLOW;
	if (*cp != ':')
		return DTERR_BAD_FORMAT;
	errno = 0;
	tm->tm_min = strtoi(cp + 1, &cp, 10);
	if (errno == ERANGE)
		return DTERR_FIELD_OVERFLOW;
	if (*cp == '\0')
	{
		tm->tm_sec = 0;
		*fsec = 0;
		/* If it's a MINUTE TO SECOND interval, take 2 fields as being mm:ss */
		if (range == (INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND)))
		{
			tm->tm_sec = tm->tm_min;
			tm->tm_min = tm->tm_hour;
			tm->tm_hour = 0;
		}
	}
	else if (*cp == '.')
	{
		/* always assume mm:ss.sss is MINUTE TO SECOND */
		dterr = ParseFractionalSecond(cp, fsec);
		if (dterr)
			return dterr;
		tm->tm_sec = tm->tm_min;
		tm->tm_min = tm->tm_hour;
		tm->tm_hour = 0;
	}
	else if (*cp == ':')
	{
		errno = 0;
		tm->tm_sec = strtoi(cp + 1, &cp, 10);
		if (errno == ERANGE)
			return DTERR_FIELD_OVERFLOW;
		if (*cp == '\0')
			*fsec = 0;
		else if (*cp == '.')
		{
			dterr = ParseFractionalSecond(cp, fsec);
			if (dterr)
				return dterr;
		}
		else
			return DTERR_BAD_FORMAT;
	}
	else
		return DTERR_BAD_FORMAT;

	/* do a sanity check */
#ifdef HAVE_INT64_TIMESTAMP
	if (tm->tm_hour < 0 || tm->tm_min < 0 || tm->tm_min > MINS_PER_HOUR - 1 ||
		tm->tm_sec < 0 || tm->tm_sec > SECS_PER_MINUTE ||
		*fsec < INT64CONST(0) ||
		*fsec > USECS_PER_SEC)
		return DTERR_FIELD_OVERFLOW;
#else
	if (tm->tm_hour < 0 || tm->tm_min < 0 || tm->tm_min > MINS_PER_HOUR - 1 ||
		tm->tm_sec < 0 || tm->tm_sec > SECS_PER_MINUTE ||
		*fsec < 0 || *fsec > 1)
		return DTERR_FIELD_OVERFLOW;
#endif

	return 0;
}


