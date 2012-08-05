
#include "datizo.h"

/*
 * Given a pointer into a time zone string, extract a number of seconds,
 * in hh[:mm[:ss]] form, from the string.
 * If any error occurs, return NULL.
 * Otherwise, return a pointer to the first character not part of the number
 * of seconds.
 */
const char *
getsecs(const char *strp, long *secsp)
{
	int			num;

	/*
	 * `HOURSPERDAY * DAYSPERWEEK - 1' allows quasi-Posix rules like
	 * "M10.4.6/26", which does not conform to Posix, but which specifies the
	 * equivalent of ``02:00 on the first Sunday on or after 23 Oct''.
	 */
	strp = getnum(strp, &num, 0, HOURSPERDAY * DAYSPERWEEK - 1);
	if (strp == NULL)
		return NULL;
	*secsp = num * (long) SECSPERHOUR;
	if (*strp == ':')
	{
		++strp;
		strp = getnum(strp, &num, 0, MINSPERHOUR - 1);
		if (strp == NULL)
			return NULL;
		*secsp += num * SECSPERMIN;
		if (*strp == ':')
		{
			++strp;
			/* `SECSPERMIN' allows for leap seconds. */
			strp = getnum(strp, &num, 0, SECSPERMIN);
			if (strp == NULL)
				return NULL;
			*secsp += num;
		}
	}
	return strp;
}


