

#include "datizo.h"

/*
 * j2day - convert Julian date to day-of-week (0..6 == Sun..Sat)
 *
 * Note: various places use the locution j2day(date - 1) to produce a
 * result according to the convention 0..6 = Mon..Sun.	This is a bit of
 * a crock, but will work as long as the computation here is just a modulo.
 */
int
j2day(int date)
{
	unsigned int day;

	day = date;

	day += 1;
	day %= 7;

	return (int) day;
}	/* j2day() */

