
#include "datizo.h"

/* ClearPgTM
 *
 * Zero out a pg_tm and associated fsec_t
 */
inline void
ClearPgTm(struct tm * tm, fsec_t *fsec)
{
	tm->tm_year = 0;
	tm->tm_mon = 0;
	tm->tm_mday = 0;
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	*fsec = 0;
}

