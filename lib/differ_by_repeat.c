/* postgresql: src/timezone/localtime.c */

#include "datizo.h"

int
differ_by_repeat(pg_time_t t1, pg_time_t t0)
{
	if (TYPE_INTEGRAL(pg_time_t) &&
		TYPE_BIT(pg_time_t) -TYPE_SIGNED(pg_time_t) <SECSPERREPEAT_BITS)
		return 0;
	return t1 - t0 == SECSPERREPEAT;
}


