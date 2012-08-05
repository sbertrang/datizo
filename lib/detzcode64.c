
#include "datizo.h"

pg_time_t
detzcode64(const char *codep)
{
	pg_time_t	result;
	int			i;

	result = (codep[0] & 0x80) ? (~(int64_t) 0) : 0;
	for (i = 0; i < 8; ++i)
		result = result * 256 + (codep[i] & 0xff);
	return result;
}


