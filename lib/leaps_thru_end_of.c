
#include "datizo.h"

/*
 * Return the number of leap years through the end of the given year
 * where, to make the math easy, the answer for year zero is defined as zero.
 */
int
leaps_thru_end_of(const int y)
{
	return (y >= 0) ? (y / 4 - y / 100 + y / 400) :
		-(leaps_thru_end_of(-(y + 1)) + 1);
}

