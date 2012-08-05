
#include "datizo.h"

/*
 *	Adjust interval for specified precision, in both YEAR to SECOND
 *	range and sub-second precision.
 */
void
AdjustIntervalForTypmod(Interval *interval, int32_t typmod)
{
#ifdef HAVE_INT64_TIMESTAMP
	static const int64_t IntervalScales[MAX_INTERVAL_PRECISION + 1] = {
		INT64CONST(1000000),
		INT64CONST(100000),
		INT64CONST(10000),
		INT64CONST(1000),
		INT64CONST(100),
		INT64CONST(10),
		INT64CONST(1)
	};

	static const int64_t IntervalOffsets[MAX_INTERVAL_PRECISION + 1] = {
		INT64CONST(500000),
		INT64CONST(50000),
		INT64CONST(5000),
		INT64CONST(500),
		INT64CONST(50),
		INT64CONST(5),
		INT64CONST(0)
	};
#else
	static const double IntervalScales[MAX_INTERVAL_PRECISION + 1] = {
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000
	};
#endif

	/*
	 * Unspecified range and precision? Then not necessary to adjust. Setting
	 * typmod to -1 is the convention for all data types.
	 */
	if (typmod >= 0)
	{
		int			range = INTERVAL_RANGE(typmod);
		int			precision = INTERVAL_PRECISION(typmod);

		/*
		 * Our interpretation of intervals with a limited set of fields is
		 * that fields to the right of the last one specified are zeroed out,
		 * but those to the left of it remain valid.  Thus for example there
		 * is no operational difference between INTERVAL YEAR TO MONTH and
		 * INTERVAL MONTH.	In some cases we could meaningfully enforce that
		 * higher-order fields are zero; for example INTERVAL DAY could reject
		 * nonzero "month" field.  However that seems a bit pointless when we
		 * can't do it consistently.  (We cannot enforce a range limit on the
		 * highest expected field, since we do not have any equivalent of
		 * SQL's <interval leading field precision>.)  If we ever decide to
		 * revisit this, interval_transform will likely require adjusting.
		 *
		 * Note: before PG 8.4 we interpreted a limited set of fields as
		 * actually causing a "modulo" operation on a given value, potentially
		 * losing high-order as well as low-order information.	But there is
		 * no support for such behavior in the standard, and it seems fairly
		 * undesirable on data consistency grounds anyway.	Now we only
		 * perform truncation or rounding of low-order fields.
		 */
		if (range == INTERVAL_FULL_RANGE)
		{
			/* Do nothing... */
		}
		else if (range == INTERVAL_MASK(YEAR))
		{
			interval->month = (interval->month / MONTHS_PER_YEAR) * MONTHS_PER_YEAR;
			interval->day = 0;
			interval->time = 0;
		}
		else if (range == INTERVAL_MASK(MONTH))
		{
			interval->day = 0;
			interval->time = 0;
		}
		/* YEAR TO MONTH */
		else if (range == (INTERVAL_MASK(YEAR) | INTERVAL_MASK(MONTH)))
		{
			interval->day = 0;
			interval->time = 0;
		}
		else if (range == INTERVAL_MASK(DAY))
		{
			interval->time = 0;
		}
		else if (range == INTERVAL_MASK(HOUR))
		{
#ifdef HAVE_INT64_TIMESTAMP
			interval->time = (interval->time / USECS_PER_HOUR) *
				USECS_PER_HOUR;
#else
			interval->time = ((int) (interval->time / SECS_PER_HOUR)) * (double) SECS_PER_HOUR;
#endif
		}
		else if (range == INTERVAL_MASK(MINUTE))
		{
#ifdef HAVE_INT64_TIMESTAMP
			interval->time = (interval->time / USECS_PER_MINUTE) *
				USECS_PER_MINUTE;
#else
			interval->time = ((int) (interval->time / SECS_PER_MINUTE)) * (double) SECS_PER_MINUTE;
#endif
		}
		else if (range == INTERVAL_MASK(SECOND))
		{
			/* fractional-second rounding will be dealt with below */
		}
		/* DAY TO HOUR */
		else if (range == (INTERVAL_MASK(DAY) |
						   INTERVAL_MASK(HOUR)))
		{
#ifdef HAVE_INT64_TIMESTAMP
			interval->time = (interval->time / USECS_PER_HOUR) *
				USECS_PER_HOUR;
#else
			interval->time = ((int) (interval->time / SECS_PER_HOUR)) * (double) SECS_PER_HOUR;
#endif
		}
		/* DAY TO MINUTE */
		else if (range == (INTERVAL_MASK(DAY) |
						   INTERVAL_MASK(HOUR) |
						   INTERVAL_MASK(MINUTE)))
		{
#ifdef HAVE_INT64_TIMESTAMP
			interval->time = (interval->time / USECS_PER_MINUTE) *
				USECS_PER_MINUTE;
#else
			interval->time = ((int) (interval->time / SECS_PER_MINUTE)) * (double) SECS_PER_MINUTE;
#endif
		}
		/* DAY TO SECOND */
		else if (range == (INTERVAL_MASK(DAY) |
						   INTERVAL_MASK(HOUR) |
						   INTERVAL_MASK(MINUTE) |
						   INTERVAL_MASK(SECOND)))
		{
			/* fractional-second rounding will be dealt with below */
		}
		/* HOUR TO MINUTE */
		else if (range == (INTERVAL_MASK(HOUR) |
						   INTERVAL_MASK(MINUTE)))
		{
#ifdef HAVE_INT64_TIMESTAMP
			interval->time = (interval->time / USECS_PER_MINUTE) *
				USECS_PER_MINUTE;
#else
			interval->time = ((int) (interval->time / SECS_PER_MINUTE)) * (double) SECS_PER_MINUTE;
#endif
		}
		/* HOUR TO SECOND */
		else if (range == (INTERVAL_MASK(HOUR) |
						   INTERVAL_MASK(MINUTE) |
						   INTERVAL_MASK(SECOND)))
		{
			/* fractional-second rounding will be dealt with below */
		}
		/* MINUTE TO SECOND */
		else if (range == (INTERVAL_MASK(MINUTE) |
						   INTERVAL_MASK(SECOND)))
		{
			/* fractional-second rounding will be dealt with below */
		}
		else
			warnx("unrecognized interval typmod: %d", typmod);

		/* Need to adjust subsecond precision? */
		if (precision != INTERVAL_FULL_PRECISION)
		{
			if (precision < 0 || precision > MAX_INTERVAL_PRECISION)
				warnx("interval(%d) precision must be between %d and %d",
						  precision, 0, MAX_INTERVAL_PRECISION);

			/*
			 * Note: this round-to-nearest code is not completely consistent
			 * about rounding values that are exactly halfway between integral
			 * values.	On most platforms, rint() will implement
			 * round-to-nearest-even, but the integer code always rounds up
			 * (away from zero).  Is it worth trying to be consistent?
			 */
#ifdef HAVE_INT64_TIMESTAMP
			if (interval->time >= INT64CONST(0))
			{
				interval->time = ((interval->time +
								   IntervalOffsets[precision]) /
								  IntervalScales[precision]) *
					IntervalScales[precision];
			}
			else
			{
				interval->time = -(((-interval->time +
									 IntervalOffsets[precision]) /
									IntervalScales[precision]) *
								   IntervalScales[precision]);
			}
#else
			interval->time = rint(((double) interval->time) *
								  IntervalScales[precision]) /
				IntervalScales[precision];
#endif
		}
	}
}


