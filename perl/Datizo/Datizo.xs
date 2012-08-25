
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <fcntl.h>
#include <unistd.h>


#include <datizo.h>

const char *
getlocaltz(void)
{
	static char	 path[MAXPATHLEN];
	const char	*tz;

	/* without a link use GMT */
	if (readlink("/etc/localtime", &path, sizeof(path)) == -1)
		return NULL;

	/* use GMT if there is no matching substring */
	if ((tz = strstr(path, "/zoneinfo/")) == NULL)
		return NULL;

	/* move behind directory parts: strlen("/zoneinfo/") */
	tz += 10;

	return tz;
}

typedef	DateADT		Datizo_Date;
typedef	Interval *	Datizo_Interval;
typedef	TimeADT		Datizo_Time;
typedef	TimeTzADT *	Datizo_TimeTz;
typedef	Timestamp	Datizo_Timestamp;
typedef	TimestampTz	Datizo_TimestampTz;
typedef	pg_tz *		Datizo_Timezone;

MODULE = Datizo		PACKAGE = Datizo		

BOOT:
{
	const char *ltz = getlocaltz();

	/* ensure default timezone and logging setting */
	pg_timezone_initialize();

	if (ltz != NULL)
		session_timezone = pg_tzset(ltz);
}

MODULE = Datizo		PACKAGE = Datizo::Interval		

Datizo_Interval
parse(SV *self, const char *string)
CODE:
	RETVAL = interval_in(string);
OUTPUT:
	RETVAL

Datizo_Interval
justify(Datizo_Interval span)
CODE:
	RETVAL = interval_justify_interval(span);
OUTPUT:
	RETVAL

char *
to_string(Datizo_Interval interval, ...)
CODE:
	RETVAL = interval_out(interval);
OUTPUT:
	RETVAL

MODULE = Datizo		PACKAGE = Datizo::TimestampTz		

Datizo_TimestampTz
now(SV *self)
CODE:
	RETVAL = GetCurrentTimestamp();
OUTPUT:
	RETVAL

Datizo_TimestampTz
parse(SV *self, char *string)
CODE:
	RETVAL = timestamptz_in(string);
OUTPUT:
	RETVAL

char *
to_string(Datizo_TimestampTz self, ...)
CODE:
	RETVAL = timestamptz_out(self);
OUTPUT:
	RETVAL

Datizo_Timezone
timezone(SV *self, const char *name)
PROTOTYPE: $;$
CODE:
	if (name == NULL)
		RETVAL = session_timezone;
	else
		RETVAL = session_timezone = pg_tzset(name);
OUTPUT:
	RETVAL

Datizo_TimestampTz
add(Datizo_TimestampTz ts, Datizo_Interval span)
CODE:
	RETVAL = timestamptz_pl_interval(ts, span);
OUTPUT:
	RETVAL

MODULE = Datizo		PACKAGE = Datizo::Timezone		

char *
to_string(Datizo_Timezone tz, ...)
CODE:
	RETVAL = tz->TZname;
OUTPUT:
	RETVAL

