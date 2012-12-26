/* postgresql: src/timezone/pgtz.c */

#include <dirent.h>
#include <string.h>

#include "datizo.h"

/*
 * Scan specified directory for a case-insensitive match to fname
 * (of length fnamelen --- fname may not be null terminated!).	If found,
 * copy the actual filename into canonname and return true.
 */
bool
scan_directory_ci(const char *dirname, const char *fname, int fnamelen, char *canonname, int canonnamelen)
{
	bool		found = false;
	DIR		   *dirdesc;
	struct dirent *direntry;

	dirdesc = opendir(dirname);
	if (!dirdesc)
	{
		warnx("could not open directory \"%s\": %m", dirname);
		return false;
	}

	while ((direntry = readdir(dirdesc)) != NULL)
	{
		/*
		 * Ignore . and .., plus any other "hidden" files.	This is a security
		 * measure to prevent access to files outside the timezone directory.
		 */
		if (direntry->d_name[0] == '.')
			continue;

		if (strlen(direntry->d_name) == fnamelen &&
			pg_strncasecmp(direntry->d_name, fname, fnamelen) == 0)
		{
			/* Found our match */
			strlcpy(canonname, direntry->d_name, canonnamelen);
			found = true;
			break;
		}
	}

	closedir(dirdesc);

	return found;
}


