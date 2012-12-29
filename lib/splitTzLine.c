/* postgresql: src/backend/utils/misc/tzparser.c */

#include "datizo.h"

bool
splitTzLine(const char *filename, int lineno, char *line, tzEntry *tzentry)
{
	char	   *abbrev;
	char	   *offset;
	char	   *offset_endptr;
	char	   *remain;
	char	   *is_dst;

	tzentry->lineno = lineno;
	tzentry->filename = filename;

	abbrev = strtok(line, WHITESPACE);
	if (!abbrev)
	{
		warnx("missing time zone abbreviation in time zone file \"%s\", line %d",
						 filename, lineno);
		return false;
	}
	tzentry->abbrev = abbrev;

	offset = strtok(NULL, WHITESPACE);
	if (!offset)
	{
		warnx("missing time zone offset in time zone file \"%s\", line %d",
						 filename, lineno);
		return false;
	}
	tzentry->offset = strtol(offset, &offset_endptr, 10);
	if (offset_endptr == offset || *offset_endptr != '\0')
	{
		warnx("invalid number for time zone offset in time zone file \"%s\", line %d",
						 filename, lineno);
		return false;
	}

	is_dst = strtok(NULL, WHITESPACE);
	if (is_dst && pg_strcasecmp(is_dst, "D") == 0)
	{
		tzentry->is_dst = true;
		remain = strtok(NULL, WHITESPACE);
	}
	else
	{
		/* there was no 'D' dst specifier */
		tzentry->is_dst = false;
		remain = is_dst;
	}

	if (!remain)				/* no more non-whitespace chars */
		return true;

	if (remain[0] != '#')		/* must be a comment */
	{
		warnx("invalid syntax in time zone file \"%s\", line %d",
						 filename, lineno);
		return false;
	}
	return true;
}

