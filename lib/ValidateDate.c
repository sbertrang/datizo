
#include "datizo.h"

/* ValidateDate()
 * Check valid year/month/day values, handle BC and DOY cases
 * Return 0 if okay, a DTERR code if not.
 */
int
ValidateDate(int fmask, bool isjulian, bool is2digits, bool bc,
			 struct tm * tm)
{
	if (fmask & DTK_M(YEAR))
	{
		if (isjulian)
		{
			/* tm_year is correct and should not be touched */
		}
		else if (bc)
		{
			/* there is no year zero in AD/BC notation */
			if (tm->tm_year <= 0)
				return DTERR_FIELD_OVERFLOW;
			/* internally, we represent 1 BC as year zero, 2 BC as -1, etc */
			tm->tm_year = -(tm->tm_year - 1);
		}
		else if (is2digits)
		{
			/* process 1 or 2-digit input as 1970-2069 AD, allow '0' and '00' */
			if (tm->tm_year < 0)	/* just paranoia */
				return DTERR_FIELD_OVERFLOW;
			if (tm->tm_year < 70)
				tm->tm_year += 2000;
			else if (tm->tm_year < 100)
				tm->tm_year += 1900;
		}
		else
		{
			/* there is no year zero in AD/BC notation */
			if (tm->tm_year <= 0)
				return DTERR_FIELD_OVERFLOW;
		}
	}

	/* now that we have correct year, decode DOY */
	if (fmask & DTK_M(DOY))
	{
		j2date(date2j(tm->tm_year, 1, 1) + tm->tm_yday - 1,
			   &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
	}

	/* check for valid month */
	if (fmask & DTK_M(MONTH))
	{
		if (tm->tm_mon < 1 || tm->tm_mon > MONTHS_PER_YEAR)
			return DTERR_MD_FIELD_OVERFLOW;
	}

	/* minimal check for valid day */
	if (fmask & DTK_M(DAY))
	{
		if (tm->tm_mday < 1 || tm->tm_mday > 31)
			return DTERR_MD_FIELD_OVERFLOW;
	}

	if ((fmask & DTK_DATE_M) == DTK_DATE_M)
	{
		/*
		 * Check for valid day of month, now that we know for sure the month
		 * and year.  Note we don't use MD_FIELD_OVERFLOW here, since it seems
		 * unlikely that "Feb 29" is a YMD-order error.
		 */
		if (tm->tm_mday > day_tab[isleap(tm->tm_year)][tm->tm_mon - 1])
			return DTERR_FIELD_OVERFLOW;
	}

	return 0;
}


