
#include "datizo.h"


/*
 * Simplified normalize logic courtesy Paul Eggert.
 */

int
increment_overflow(int *number, int delta)
{
	int			number0;

	number0 = *number;
	*number += delta;
	return (*number < number0) != (delta < 0);
}

