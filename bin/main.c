
#include <err.h>
#include <sysexits.h>

#include "datizo.h"

int
main(int argc, char *argv [])
{
	TimestampTz	 tsz;
	char		*s;

	pg_timezone_initialize();

	warnx("test...");

	/*
	pg_tzset("Europe/Amsterdam");
	*/

	tsz = timestamptz_in("2012/1/1 00:20:01+05");

	s = timestamptz_out(tsz);

	warnx("s: %s", s);

	return EX_OK;
}

