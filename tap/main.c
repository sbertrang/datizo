
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

enum tapresult {
	TAP_PASS,
	TAP_FAIL,
	TAP_SKIP,
	TAP_TODO
};

struct tapline {
	TAILQ_ENTRY(tapline)	 entry;
	char		*group;
	char		*label;
	enum tapresult	 result;
	char		*string;
	char		*output;
	char		*comment;
};

TAILQ_HEAD(taplines, tapline);

struct tapcol {
	char	*name;
	int	 optional;
	size_t	 offset;
} tapcols[] = {
	{ "group",	0, offsetof(struct tapline,group) },
	{ "label",	0, offsetof(struct tapline,label) },
	{ "string",	0, offsetof(struct tapline,string) },
	{ "result",	0, offsetof(struct tapline,result) },
	{ "output",	0, offsetof(struct tapline,output) },
	{ "comment",	1, offsetof(struct tapline,comment) }
};
#define	TAPCOLS	(sizeof(tapcols)/sizeof(tapcols[0]))

struct tapline *
clonetapline(const struct tapline *src)
{
	struct tapline *tapline;

	if (src->group == NULL || src->label == NULL || src->string == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if ((tapline = malloc(sizeof(*tapline))) == NULL)
		return NULL;

	tapline->group = strdup(src->group);
	tapline->label = strdup(src->label);
	tapline->string = strdup(src->string);
	tapline->result = src->result;
	tapline->output = strdup(src->output);
	tapline->comment = src->comment == NULL || src->comment[0] == '\0' ? NULL : strdup(src->comment);

	return tapline;
}

/*
 * rfc1123	ihosts	Sun, 06 Nov 1994 08:49:37 GMT	pass	1994-11-06 09:49:37+01	
 */

int
parseline(struct taplines *taplines, char *line, size_t len)
{
	char		 buf[len+1];
	char		*p;
	char		*r;
	char		**s;
	size_t		 n;
	size_t		 rlen;
	struct tapline	 tmpline;
	struct tapline	*tapline;
	struct tapcol	*col;
	int		 c;

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

	/* skip comment lines */
	if (buf[0] == '#') {
		return 0;
	}

	/* warnx("line: %s", buf); */

	p = buf;
	n = 0;
	while ((r = strsep(&p, "\t")) != NULL) {
		if (n >= TAPCOLS)
			err(0, "n=%zd > TAPCOLS=%d", n, TAPCOLS);

		rlen = strlen(r);
		col = &tapcols[n];

		if (col->optional == 1 && rlen == 0)
			continue;

		/* warnx("r: %s (%zd) [%zd:%s,%d]", r, rlen, n, col->name, col->optional); */

		if (strcmp(col->name, "result") == 0) {
			if (strcasecmp(r, "pass") == 0)
				tmpline.result = TAP_PASS;
			else if (strcasecmp(r, "fail") == 0)
				tmpline.result = TAP_FAIL;
			else if (strcasecmp(r, "skip") == 0)
				tmpline.result = TAP_SKIP;
			else if (strcasecmp(r, "todo") == 0)
				tmpline.result = TAP_TODO;
			else {
				warnx("unknown test result: %s", r);
				return -1;
			}
		}
		else
			*(char **)((char *)&tmpline + col->offset) = r;

		n++;
	}

	tapline = clonetapline(&tmpline);

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

	plan_tests(lines * 2);

	pg_timezone_initialize();
	session_timezone = pg_tzset("Europe/Amsterdam");

	TAILQ_FOREACH(tapline, &taplines, entry) {
		/* warnx("# %s - %s # %s\n%s", tapline->group, tapline->label, tapline->comment ? tapline->comment : "", tapline->string); */


		if (tapline->result == TAP_TODO)
			todo_start("expect fail on: %s", tapline->string);

		if (tapline->result == TAP_SKIP)
			skip(2, "ignore: %s", tapline->string);
		else {
			char *s;
			tsz = timestamptz_in(tapline->string);
			ok(tsz != NULL, "%s - %s: %s", tapline->group, tapline->label, tapline->string);

			s = timestamptz_out(tsz);
			ok( strcmp( s, tapline->output ) == 0, "output matches: <%s> == <%s>", s, tapline->output );
		}

		if (tapline->result == TAP_TODO)
			todo_end();
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

	return exit_status();
}

