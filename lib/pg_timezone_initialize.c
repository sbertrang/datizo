/* postgresql: src/timezone/pgtz.c */

#include "datizo.h"

/* Current session timezone (controlled by TimeZone GUC) */
pg_tz	   *session_timezone = NULL;

/* Current log timezone (controlled by log_timezone GUC) */
pg_tz	   *log_timezone = NULL;



/*
 * Initialize timezone library
 *
 * This is called before GUC variable initialization begins.  Its purpose
 * is to ensure that log_timezone has a valid value before any logging GUC
 * variables could become set to values that require elog.c to provide
 * timestamps (e.g., log_line_prefix).  We may as well initialize
 * session_timestamp to something valid, too.
 */
void
pg_timezone_initialize(void)
{
	/*
	 * We may not yet know where PGSHAREDIR is (in particular this is true in
	 * an EXEC_BACKEND subprocess).  So use "GMT", which pg_tzset forces to
	 * be interpreted without reference to the filesystem.  This corresponds
	 * to the bootstrap default for these variables in guc.c, although in
	 * principle it could be different.
	 */
	session_timezone = pg_tzset("GMT");
	log_timezone = session_timezone;
}


