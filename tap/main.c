
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <sysexits.h>
#include <tap.h>

#include <fcntl.h>
/*
 * open()
 */

#include <stdio.h>
/*
 * read()
 */
#include <sys/queue.h>
#include <string.h>


#include "datizo.h"

struct tapline {
	TAILQ_ENTRY(tapline)	 entry;
	char		*group;
	char		*label;
	char		*string;
	char		*comment;
};

TAILQ_HEAD(taplines, tapline);

struct tapline *
mktapline(const char *group, const char *label, const char *string, const char *comment)
{
	struct tapline *tapline;

	if (group == NULL || label == NULL || string == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((tapline = malloc(sizeof(*tapline))) == NULL)
		return NULL;

	tapline->group = strdup(group);
	tapline->label = strdup(label);
	tapline->string = strdup(string);
	tapline->comment = comment == NULL ? NULL : strdup(comment);

	return tapline;
}

int
parseline(struct taplines *taplines, char *line, size_t len)
{
	char	 buf[len+1];
	char	*p;
	size_t	 n;
	struct tapline	  tmpline;
	struct tapline	 *tapline;

	bzero(&tmpline, sizeof(tmpline));

	strlcpy(buf, line, len+1);
	buf[len] = '\0';

	if (buf[len-1] == '\n') {
		buf[len-1] = '\0';
		len--;
	}
	if (buf[len-1] == '\r') {
		buf[len-1] = '\0';
		len--;
	}

	if (len == 0)
		return 0;

	/*
	warnx("buf: <%s> (%zd)", buf, len);
	*/

	p = buf;
	if ((n = strcspn(p, "\t")) == 0) {
		warnx("no tab after group");
		return -1;
	}
	if (p[n] != '\t') {
		warnx("really no tab after group");
		return -1;
	}

	p[n] = '\0';
	/*
	warnx("group: %s", p);
	*/
	tmpline.group = p;
	p += n+1;

	if ((n = strcspn(p, "\t")) == 0) {
		warnx("no tab after label");
		return -1;
	}
	if (p[n] != '\t') {
		warnx("really no tab after label");
		return -1;
	}
	p[n] = '\0';
	tmpline.label = p;
	/*
	warnx("label: %s", p);
	*/
	p += n+1;


	if ((n = strcspn(p, "\t")) == 0) {
		warnx("no tab after date");
		return -1;
	}
	p[n] = '\0';
	tmpline.string = p;
	/*
	warnx("date: %s", p);
	*/
	p += n+1;

	/* can be done after the date */
	if (p[0] == '\0') {
		tapline = mktapline(tmpline.group, tmpline.label, tmpline.string, NULL);
		TAILQ_INSERT_TAIL(taplines, tapline, entry);
		return 1;
	}

	/* allow multiple tabs before comment */
	n = 0;
	while (p[n] == '\t')
		n++;

	if (p[n] != '#') {
		warnx("something which is not a hash: <%s>", p);
		return -1;
	}

	/* skip leading tabs and hash sign */
	p += n+1;

	n = strspn(p, " \t");
	p += n;

	if (p[0] == '\0')
		return -1;

	/*
	warnx("comment: %s", p);
	*/
	tmpline.comment = p;

	tapline = mktapline(tmpline.group, tmpline.label, tmpline.string, tmpline.comment);

	TAILQ_INSERT_TAIL(taplines, tapline, entry);

	return 1;
}

int
getdates(const char *path)
{
	FILE	*fp;
	char	*line;
	size_t	 len;
	int	 next = 1;
	struct taplines	 taplines;
	int	 lines = 0;
	struct tapline	*tapline;
	TimestampTz	 tsz;
	int	 n;

	TAILQ_INIT(&taplines);

	if ((fp = fopen(path, "r")) == NULL) {
		warn("fopen");
		return -1;
	}

	while (next) {
		if ((line = fgetln(fp, &len)) == NULL) {
			if (!feof(fp)) {
				warn("fgetln");
				return -1;
			}
			break;
		}

		if ((n = parseline(&taplines, line, len)) == -1) {
			warn("parseline");
			return -1;
		}
		if (n == 1)
			lines++;
	}

	diag("Found %d test cases...", lines);

	plan_tests(lines);

	pg_timezone_initialize();
	session_timezone = pg_tzset("Europe/Amsterdam");

	TAILQ_FOREACH(tapline, &taplines, entry) {
		/* warnx("# %s - %s # %s\n%s", tapline->group, tapline->label, tapline->comment ? tapline->comment : "", tapline->string); */

		tsz = timestamptz_in(tapline->string);
		ok(tsz != NULL, "%s - %s: %s", tapline->group, tapline->label, tapline->string);
	}

	return 0;
}

int
main(int argc, char *argv [])
{
	const char	*file = "dates";

	if (getdates(file) == -1) {
		warnx("getdates fail: %s", file);
		exit(EX_USAGE);
	}

#if 0
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
	
# endif

	return exit_status();
}

