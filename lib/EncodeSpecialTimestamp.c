/* postgresql: src/backend/utils/adt/timestamp.c */

#include <string.h>

#include "datizo.h"

/* EncodeSpecialTimestamp()
 * Convert reserved timestamp data type to string.
 */
void
EncodeSpecialTimestamp(Timestamp dt, char *str)
{
	if (TIMESTAMP_IS_NOBEGIN(dt))
		strcpy(str, EARLY);
	else if (TIMESTAMP_IS_NOEND(dt))
		strcpy(str, LATE);
	else	/* shouldn't happen */
		warnx("invalid argument for EncodeSpecialTimestamp");
}



