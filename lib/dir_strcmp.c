/* postgresql: src/port/path.c */

#include "datizo.h"

/*
 */
int
dir_strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2)
	{
		if (
#ifndef WIN32
			*s1 != *s2
#else
		/* On windows, paths are case-insensitive */
			pg_tolower((unsigned char) *s1) != pg_tolower((unsigned char) *s2)
#endif
			&& !(IS_DIR_SEP(*s1) && IS_DIR_SEP(*s2)))
			return (int) *s1 - (int) *s2;
		s1++, s2++;
	}
	if (*s1)
		return 1;				/* s1 longer */
	if (*s2)
		return -1;				/* s2 longer */
	return 0;
}
