
#include <err.h>
#include <sysexits.h>
#include <tap.h>

#include "datizo.h"

int
main(int argc, char *argv [])
{
	TimestampTz	 tsz, eol;
	Timestamp	 ts;
	Interval	*i;
	char		*s;
	DateADT		 d;
	TimeADT		 time;
	TimeTzADT	*timetz;
	int		 rc = 0;

	pg_timezone_initialize();

	diag("test...");

	rc = plan_tests(10);

	session_timezone = pg_tzset("Europe/Amsterdam");
	rc = ok( session_timezone != NULL, "session timezone defined" );

	tsz = timestamptz_in("2012/1/1 00:20:01+05");
	rc = ok( tsz != 0, "timestamp with timezone defined" );

	s = timestamptz_out(tsz);
	rc = ok( s != NULL, "timestamp as string: %s", s );
	rc = ok( strcmp( s, "2011-12-31 20:20:01+01" ) == 0, "timestamp matches format" );

	session_timezone = pg_tzset("Canada/Mountain");
	rc = ok( session_timezone != NULL, "switch to canada timezone" );
	s = timestamptz_out(tsz);
	rc = ok( s != NULL, "timestamp as string: %s", s );
	rc = ok( strcmp( s, "2011-12-31 12:20:01-07" ) == 0, "timestamp in canada" );


	session_timezone = pg_tzset("Indian/Maldives");
	rc = ok( session_timezone != NULL, "switch to maldives timezone" );
	s = timestamptz_out(tsz);
	rc = ok( s != NULL, "timestamp as string: %s", s );
	rc = ok( strcmp( s, "2012-01-01 00:20:01+05" ) == 0, "timestamp in indian maldives" );
	

	return exit_status();
}

