
#include <err.h>
#include <sysexits.h>

#include "datizo.h"

int
main(int argc, char *argv [])
{
	TimeZoneAbbrevTable	*tzabbr;
	TimestampTz		 tsz, eol;
	Timestamp		 ts;
	TimeTzADT		*timetz;
	Interval		*i;
	DateADT			 d;
	TimeADT			 time;
	char			*s;

	pg_timezone_initialize();

	if (!(tzabbr = load_tzoffsets("Default")))
		warnx("failed to load Default");
	else
		InstallTimeZoneAbbrevs(tzabbr);

	warnx("test...");

	session_timezone = pg_tzset("Europe/Amsterdam");

	tsz = timestamptz_in("2012/1/1 00:20:01+05");
	s = timestamptz_out(tsz);

	warnx("timestamptz: %s", s);

	ts = timestamp_in("2012/12/12 12:12:12");
	s = timestamp_out(ts);

	warnx("timestamp: %s", s);

	i = interval_in("10h5m");
	s = interval_out(i);

	warnx("interval: %s", s);



	i = interval_in("1000000 hours");
	s = interval_out(interval_justify_interval(i));

	warnx("lifetime: %s", s);


	d = date_in("1999/9/9");
	s = date_out(d);

	warnx("date: %s", s);


	time = time_in("10:20");
	s = time_out(time);

	warnx("time: %s", s);

	timetz = timetz_in("10:20");
	s = timetz_out(timetz);

	warnx("timetz: %s", s);

	tsz = GetCurrentTimestamp();
	s = timestamptz_out(tsz);

	warnx("timestamptz: %s (now)", s);


	eol = timestamptz_pl_interval(tsz, interval_justify_interval(i));
	s = timestamptz_out(eol);

	warnx("eol: %s", s);


	session_timezone = pg_tzset("Canada/Mountain");

	s = timestamptz_out(tsz);

	warnx("timestamptz in canada: %s", s);

	return EX_OK;
}

