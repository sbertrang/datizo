/* postgresql: src/backend/utils/adt/datetime.c */

#include <limits.h>
#include <stdlib.h>

/*
 * strtoi --- just like strtol, but returns int not long
 */
int
strtoi(const char *nptr, char **endptr, int base)
{
	long		val;

	val = strtol(nptr, endptr, base);
#ifdef HAVE_LONG_INT_64
	if (val != (long) ((int32) val))
		errno = ERANGE;
#endif
	return (int) val;
}


