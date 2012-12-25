/* postgresql: src/backend/utils/adt/datetime.c */

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "datizo.h"

/* DecodeInterval()
 * Interpret previously parsed fields for general time interval.
 * Returns 0 if successful, DTERR code if bogus input detected.
 * dtype, tm, fsec are output parameters.
 *
 * Allow "date" field DTK_DATE since this could be just
 *	an unsigned floating point number. - thomas 1997-11-16
 *
 * Allow ISO-style time span, with implicit units on number of days
 *	preceding an hh:mm:ss field. - thomas 1998-04-30
 */
int
DecodeInterval(char **field, int *ftype, int nf, int range,
			   int *dtype, struct tm * tm, fsec_t *fsec)
{
	bool		is_before = FALSE;
	char	   *cp;
	int			fmask = 0,
				tmask,
				type;
	int			i;
	int			dterr;
	int			val;
	double		fval;

	*dtype = DTK_DELTA;
	type = IGNORE_DTF;
	ClearPgTm(tm, fsec);

	/* read through list backwards to pick up units before values */
	for (i = nf - 1; i >= 0; i--)
	{
		switch (ftype[i])
		{
			case DTK_TIME:
				dterr = DecodeTime(field[i], fmask, range,
								   &tmask, tm, fsec);
				if (dterr)
					return dterr;
				type = DTK_DAY;
				break;

			case DTK_TZ:

				/*
				 * Timezone is a token with a leading sign character and at
				 * least one digit; there could be ':', '.', '-' embedded in
				 * it as well.
				 */
				assert(*field[i] == '-' || *field[i] == '+');

				/*
				 * Try for hh:mm or hh:mm:ss.  If not, fall through to
				 * DTK_NUMBER case, which can handle signed float numbers and
				 * signed year-month values.
				 */
				if (strchr(field[i] + 1, ':') != NULL &&
					DecodeTime(field[i] + 1, fmask, INTERVAL_FULL_RANGE,
							   &tmask, tm, fsec) == 0)
				{
					if (*field[i] == '-')
					{
						/* flip the sign on all fields */
						tm->tm_hour = -tm->tm_hour;
						tm->tm_min = -tm->tm_min;
						tm->tm_sec = -tm->tm_sec;
						*fsec = -(*fsec);
					}

					/*
					 * Set the next type to be a day, if units are not
					 * specified. This handles the case of '1 +02:03' since we
					 * are reading right to left.
					 */
					type = DTK_DAY;
					tmask = DTK_M(TZ);
					break;
				}
				/* FALL THROUGH */

			case DTK_DATE:
			case DTK_NUMBER:
				if (type == IGNORE_DTF)
				{
					/* use typmod to decide what rightmost field is */
					switch (range)
					{
						case INTERVAL_MASK(YEAR):
							type = DTK_YEAR;
							break;
						case INTERVAL_MASK(MONTH):
						case INTERVAL_MASK(YEAR) | INTERVAL_MASK(MONTH):
							type = DTK_MONTH;
							break;
						case INTERVAL_MASK(DAY):
							type = DTK_DAY;
							break;
						case INTERVAL_MASK(HOUR):
						case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR):
							type = DTK_HOUR;
							break;
						case INTERVAL_MASK(MINUTE):
						case INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE):
						case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE):
							type = DTK_MINUTE;
							break;
						case INTERVAL_MASK(SECOND):
						case INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
						case INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
						case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
							type = DTK_SECOND;
							break;
						default:
							type = DTK_SECOND;
							break;
					}
				}

				errno = 0;
				val = strtoi(field[i], &cp, 10);
				if (errno == ERANGE)
					return DTERR_FIELD_OVERFLOW;

				if (*cp == '-')
				{
					/* SQL "years-months" syntax */
					int			val2;

					val2 = strtoi(cp + 1, &cp, 10);
					if (errno == ERANGE || val2 < 0 || val2 >= MONTHS_PER_YEAR)
						return DTERR_FIELD_OVERFLOW;
					if (*cp != '\0')
						return DTERR_BAD_FORMAT;
					type = DTK_MONTH;
					if (*field[i] == '-')
						val2 = -val2;
					val = val * MONTHS_PER_YEAR + val2;
					fval = 0;
				}
				else if (*cp == '.')
				{
					errno = 0;
					fval = strtod(cp, &cp);
					if (*cp != '\0' || errno != 0)
						return DTERR_BAD_FORMAT;

					if (*field[i] == '-')
						fval = -fval;
				}
				else if (*cp == '\0')
					fval = 0;
				else
					return DTERR_BAD_FORMAT;

				tmask = 0;		/* DTK_M(type); */

				switch (type)
				{
					case DTK_MICROSEC:
#ifdef HAVE_INT64_TIMESTAMP
						*fsec += rint(val + fval);
#else
						*fsec += (val + fval) * 1e-6;
#endif
						tmask = DTK_M(MICROSECOND);
						break;

					case DTK_MILLISEC:
						/* avoid overflowing the fsec field */
						tm->tm_sec += val / 1000;
						val -= (val / 1000) * 1000;
#ifdef HAVE_INT64_TIMESTAMP
						*fsec += rint((val + fval) * 1000);
#else
						*fsec += (val + fval) * 1e-3;
#endif
						tmask = DTK_M(MILLISECOND);
						break;

					case DTK_SECOND:
						tm->tm_sec += val;
#ifdef HAVE_INT64_TIMESTAMP
						*fsec += rint(fval * 1000000);
#else
						*fsec += fval;
#endif

						/*
						 * If any subseconds were specified, consider this
						 * microsecond and millisecond input as well.
						 */
						if (fval == 0)
							tmask = DTK_M(SECOND);
						else
							tmask = DTK_ALL_SECS_M;
						break;

					case DTK_MINUTE:
						tm->tm_min += val;
						AdjustFractSeconds(fval, tm, fsec, SECS_PER_MINUTE);
						tmask = DTK_M(MINUTE);
						break;

					case DTK_HOUR:
						tm->tm_hour += val;
						AdjustFractSeconds(fval, tm, fsec, SECS_PER_HOUR);
						tmask = DTK_M(HOUR);
						type = DTK_DAY; /* set for next field */
						break;

					case DTK_DAY:
						tm->tm_mday += val;
						AdjustFractSeconds(fval, tm, fsec, SECS_PER_DAY);
						tmask = DTK_M(DAY);
						break;

					case DTK_WEEK:
						tm->tm_mday += val * 7;
						AdjustFractDays(fval, tm, fsec, 7);
						tmask = DTK_M(WEEK);
						break;

					case DTK_MONTH:
						tm->tm_mon += val;
						AdjustFractDays(fval, tm, fsec, DAYS_PER_MONTH);
						tmask = DTK_M(MONTH);
						break;

					case DTK_YEAR:
						tm->tm_year += val;
						if (fval != 0)
							tm->tm_mon += fval * MONTHS_PER_YEAR;
						tmask = DTK_M(YEAR);
						break;

					case DTK_DECADE:
						tm->tm_year += val * 10;
						if (fval != 0)
							tm->tm_mon += fval * MONTHS_PER_YEAR * 10;
						tmask = DTK_M(DECADE);
						break;

					case DTK_CENTURY:
						tm->tm_year += val * 100;
						if (fval != 0)
							tm->tm_mon += fval * MONTHS_PER_YEAR * 100;
						tmask = DTK_M(CENTURY);
						break;

					case DTK_MILLENNIUM:
						tm->tm_year += val * 1000;
						if (fval != 0)
							tm->tm_mon += fval * MONTHS_PER_YEAR * 1000;
						tmask = DTK_M(MILLENNIUM);
						break;

					default:
						return DTERR_BAD_FORMAT;
				}
				break;

			case DTK_STRING:
			case DTK_SPECIAL:
				type = DecodeUnits(i, field[i], &val);
				if (type == IGNORE_DTF)
					continue;

				tmask = 0;		/* DTK_M(type); */
				switch (type)
				{
					case UNITS:
						type = val;
						break;

					case AGO:
						is_before = TRUE;
						type = val;
						break;

					case RESERV:
						tmask = (DTK_DATE_M | DTK_TIME_M);
						*dtype = val;
						break;

					default:
						return DTERR_BAD_FORMAT;
				}
				break;

			default:
				return DTERR_BAD_FORMAT;
		}

		if (tmask & fmask)
			return DTERR_BAD_FORMAT;
		fmask |= tmask;
	}

	/* ensure that at least one time field has been found */
	if (fmask == 0)
		return DTERR_BAD_FORMAT;

	/* ensure fractional seconds are fractional */
	if (*fsec != 0)
	{
		int			sec;

#ifdef HAVE_INT64_TIMESTAMP
		sec = *fsec / USECS_PER_SEC;
		*fsec -= sec * USECS_PER_SEC;
#else
		TMODULO(*fsec, sec, 1.0);
#endif
		tm->tm_sec += sec;
	}

	/*----------
	 * The SQL standard defines the interval literal
	 *	 '-1 1:00:00'
	 * to mean "negative 1 days and negative 1 hours", while Postgres
	 * traditionally treats this as meaning "negative 1 days and positive
	 * 1 hours".  In SQL_STANDARD intervalstyle, we apply the leading sign
	 * to all fields if there are no other explicit signs.
	 *
	 * We leave the signs alone if there are additional explicit signs.
	 * This protects us against misinterpreting postgres-style dump output,
	 * since the postgres-style output code has always put an explicit sign on
	 * all fields following a negative field.  But note that SQL-spec output
	 * is ambiguous and can be misinterpreted on load!	(So it's best practice
	 * to dump in postgres style, not SQL style.)
	 *----------
	 */
	if (IntervalStyle == INTSTYLE_SQL_STANDARD && *field[0] == '-')
	{
		/* Check for additional explicit signs */
		bool		more_signs = false;

		for (i = 1; i < nf; i++)
		{
			if (*field[i] == '-' || *field[i] == '+')
			{
				more_signs = true;
				break;
			}
		}

		if (!more_signs)
		{
			/*
			 * Rather than re-determining which field was field[0], just force
			 * 'em all negative.
			 */
			if (*fsec > 0)
				*fsec = -(*fsec);
			if (tm->tm_sec > 0)
				tm->tm_sec = -tm->tm_sec;
			if (tm->tm_min > 0)
				tm->tm_min = -tm->tm_min;
			if (tm->tm_hour > 0)
				tm->tm_hour = -tm->tm_hour;
			if (tm->tm_mday > 0)
				tm->tm_mday = -tm->tm_mday;
			if (tm->tm_mon > 0)
				tm->tm_mon = -tm->tm_mon;
			if (tm->tm_year > 0)
				tm->tm_year = -tm->tm_year;
		}
	}

	/* finally, AGO negates everything */
	if (is_before)
	{
		*fsec = -(*fsec);
		tm->tm_sec = -tm->tm_sec;
		tm->tm_min = -tm->tm_min;
		tm->tm_hour = -tm->tm_hour;
		tm->tm_mday = -tm->tm_mday;
		tm->tm_mon = -tm->tm_mon;
		tm->tm_year = -tm->tm_year;
	}

	return 0;
}


