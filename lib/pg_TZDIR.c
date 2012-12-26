/* postgresql: src/timezone/pgtz.c */

#include <sys/param.h>

#include "datizo.h"

#define	SYSTEMTZDIR "/usr/share/zoneinfo"

/*
 * Return full pathname of timezone data directory
 */
const char *
pg_TZDIR(void)
{
#ifndef SYSTEMTZDIR
	/* normal case: timezone stuff is under our share dir */
	static bool done_tzdir = false;
	static char tzdir[MAXPATHLEN];

	if (done_tzdir)
		return tzdir;

	get_share_path(my_exec_path, tzdir);
	strlcpy(tzdir + strlen(tzdir), "/timezone", MAXPATHLEN - strlen(tzdir));

	done_tzdir = true;
	return tzdir;
#else
	/* we're configured to use system's timezone database */
	return SYSTEMTZDIR;
#endif
}


