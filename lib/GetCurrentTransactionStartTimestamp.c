/* postgresql: src/backend/access/transam/xact.c */

#include "datizo.h"

static TimestampTz xactStartTimestamp;

/*
 *      GetCurrentTransactionStartTimestamp
 */
TimestampTz
GetCurrentTransactionStartTimestamp(void)
{
	return xactStartTimestamp;
}


