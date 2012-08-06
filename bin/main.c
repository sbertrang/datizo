
#include <err.h>
#include <sysexits.h>

#include "datizo.h"

int
main(int argc, char *argv [])
{
	TimestampTz	 tsz;
	Timestamp	 ts;
	Interval	*i;
	char		*s;
	DateADT		 dt;

	pg_timezone_initialize();

	warnx("test...");

	/*
	pg_tzset("Europe/Amsterdam");
	*/

	tsz = timestamptz_in("2012/1/1 00:20:01+05");
	s = timestamptz_out(tsz);

	warnx("timestamptz: %s", s);

	ts = timestamp_in("2012/12/12 12:12:12");
	s = timestamp_out(ts);

	warnx("timestamp: %s", s);

	i = interval_in("10h5m");
	s = interval_out(i);

	warnx("interval: %s", s);

	dt = date_in("1999/9/9");
	s = date_out(dt);

	warnx("date: %s", s);

	return EX_OK;
}

