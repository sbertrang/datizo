/* postgresql: src/backend/utils/adt/timestamp.c */

#include "datizo.h"

Timestamp
dt2local(Timestamp dt, int tz)
{
#ifdef HAVE_INT64_TIMESTAMP
	dt -= (tz * USECS_PER_SEC);
#else
	dt -= tz;
#endif
	return dt;
}

