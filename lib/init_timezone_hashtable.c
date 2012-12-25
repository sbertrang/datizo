/* postgresql: src/timezone/pgtz.c */

#include <string.h>

#include "datizo.h"
#include "hsearch.h"

extern HTAB *timezone_cache;




bool
init_timezone_hashtable(void)
{
	HASHCTL		hash_ctl;

	MemSet(&hash_ctl, 0, sizeof(hash_ctl));

	hash_ctl.keysize = TZ_STRLEN_MAX + 1;
	hash_ctl.entrysize = sizeof(pg_tz_cache);

	timezone_cache = hash_create("Timezones",
								 4,
								 &hash_ctl,
								 HASH_ELEM);
	if (!timezone_cache)
		return false;

	return true;
}

