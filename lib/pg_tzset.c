
#include <string.h>

#include "datizo.h"
#include "hsearch.h"

HTAB *timezone_cache = NULL;

/*
 * Load a timezone from file or from cache.
 * Does not verify that the timezone is acceptable!
 *
 * "GMT" is always interpreted as the tzparse() definition, without attempting
 * to load a definition from the filesystem.  This has a number of benefits:
 * 1. It's guaranteed to succeed, so we don't have the failure mode wherein
 * the bootstrap default timezone setting doesn't work (as could happen if
 * the OS attempts to supply a leap-second-aware version of "GMT").
 * 2. Because we aren't accessing the filesystem, we can safely initialize
 * the "GMT" zone definition before my_exec_path is known.
 * 3. It's quick enough that we don't waste much time when the bootstrap
 * default timezone setting is later overridden from postgresql.conf.
 */
pg_tz *
pg_tzset(const char *name)
{
	pg_tz_cache *tzp;
	struct state tzstate;
	char		uppername[TZ_STRLEN_MAX + 1];
	char		canonname[TZ_STRLEN_MAX + 1];
	char	   *p;

	if (strlen(name) > TZ_STRLEN_MAX)
		return NULL;			/* not going to fit */

	if (!timezone_cache)
		if (!init_timezone_hashtable())
			return NULL;

	/*
	 * Upcase the given name to perform a case-insensitive hashtable search.
	 * (We could alternatively downcase it, but we prefer upcase so that we
	 * can get consistently upcased results from tzparse() in case the name is
	 * a POSIX-style timezone spec.)
	 */
	p = uppername;
	while (*name)
		*p++ = pg_toupper((unsigned char) *name++);
	*p = '\0';

	tzp = (pg_tz_cache *) hash_search(timezone_cache,
									  uppername,
									  HASH_FIND,
									  NULL);
	if (tzp)
	{
		/* Timezone found in cache, nothing more to do */
		return &tzp->tz;
	}

	/*
	 * "GMT" is always sent to tzparse(), as per discussion above.
	 */
	if (strcmp(uppername, "GMT") == 0)
	{
		if (tzparse(uppername, &tzstate, TRUE) != 0)
		{
			/* This really, really should not happen ... */
			warnx("could not initialize GMT time zone");
		}
		/* Use uppercase name as canonical */
		strcpy(canonname, uppername);
	}
	else if (tzload(uppername, canonname, &tzstate, TRUE) != 0)
	{
		if (uppername[0] == ':' || tzparse(uppername, &tzstate, FALSE) != 0)
		{
			/* Unknown timezone. Fail our call instead of loading GMT! */
			return NULL;
		}
		/* For POSIX timezone specs, use uppercase name as canonical */
		strcpy(canonname, uppername);
	}

	/* Save timezone in the cache */
	tzp = (pg_tz_cache *) hash_search(timezone_cache,
									  uppername,
									  HASH_ENTER,
									  NULL);

	/* hash_search already copied uppername into the hash key */
	strcpy(tzp->tz.TZname, canonname);
	memcpy(&tzp->tz.state, &tzstate, sizeof(tzstate));

	return &tzp->tz;
}

