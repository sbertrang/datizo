/* postgresql: src/timezone/localtime.c */

#include "datizo.h"

long
detzcode(const char *codep)
{
	long		result;
	int			i;

	result = (codep[0] & 0x80) ? ~0L : 0;
	for (i = 0; i < 4; ++i)
		result = (result << 8) | (codep[i] & 0xff);
	return result;
}


