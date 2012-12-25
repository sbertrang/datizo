/* postgresql: src/timezone/localtime.c */

#include "datizo.h"

void
gmtload(struct state * sp)
{
	if (tzload(gmt, NULL, sp, TRUE) != 0)
		(void) tzparse(gmt, sp, TRUE);
}

