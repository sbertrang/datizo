
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


