
#include "datizo.h"

/*
 * Given a pointer into a time zone string, scan until a character that is not
 * a valid character in a zone name is found. Return a pointer to that
 * character.
 */
const char *
getzname(const char *strp)
{
	char		c;

	while ((c = *strp) != '\0' && !is_digit(c) && c != ',' && c != '-' &&
		   c != '+')
		++strp;
	return strp;
}


