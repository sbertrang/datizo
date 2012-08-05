
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "datizo.h"


/* ParseDateTime()
 *	Break string into tokens based on a date/time context.
 *	Returns 0 if successful, DTERR code if bogus input detected.
 *
 * timestr - the input string
 * workbuf - workspace for field string storage. This must be
 *	 larger than the largest legal input for this datetime type --
 *	 some additional space will be needed to NUL terminate fields.
 * buflen - the size of workbuf
 * field[] - pointers to field strings are returned in this array
 * ftype[] - field type indicators are returned in this array
 * maxfields - dimensions of the above two arrays
 * *numfields - set to the actual number of fields detected
 *
 * The fields extracted from the input are stored as separate,
 * null-terminated strings in the workspace at workbuf. Any text is
 * converted to lower case.
 *
 * Several field types are assigned:
 *	DTK_NUMBER - digits and (possibly) a decimal point
 *	DTK_DATE - digits and two delimiters, or digits and text
 *	DTK_TIME - digits, colon delimiters, and possibly a decimal point
 *	DTK_STRING - text (no digits or punctuation)
 *	DTK_SPECIAL - leading "+" or "-" followed by text
 *	DTK_TZ - leading "+" or "-" followed by digits (also eats ':', '.', '-')
 *
 * Note that some field types can hold unexpected items:
 *	DTK_NUMBER can hold date fields (yy.ddd)
 *	DTK_STRING can hold months (January) and time zones (PST)
 *	DTK_DATE can hold time zone names (America/New_York, GMT-8)
 */
int
ParseDateTime(const char *timestr, char *workbuf, size_t buflen,
			  char **field, int *ftype, int maxfields, int *numfields)
{
	int			nf = 0;
	const char *cp = timestr;
	char	   *bufp = workbuf;
	const char *bufend = workbuf + buflen;

	/*
	 * Set the character pointed-to by "bufptr" to "newchar", and increment
	 * "bufptr". "end" gives the end of the buffer -- we return an error if
	 * there is no space left to append a character to the buffer. Note that
	 * "bufptr" is evaluated twice.
	 */
#define APPEND_CHAR(bufptr, end, newchar)		\
	do											\
	{											\
		if (((bufptr) + 1) >= (end))			\
			return DTERR_BAD_FORMAT;			\
		*(bufptr)++ = newchar;					\
	} while (0)

	/* outer loop through fields */
	while (*cp != '\0')
	{
		/* Ignore spaces between fields */
		if (isspace((unsigned char) *cp))
		{
			cp++;
			continue;
		}

		/* Record start of current field */
		if (nf >= maxfields)
			return DTERR_BAD_FORMAT;
		field[nf] = bufp;

		/* leading digit? then date or time */
		if (isdigit((unsigned char) *cp))
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			while (isdigit((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, *cp++);

			/* time field? */
			if (*cp == ':')
			{
				ftype[nf] = DTK_TIME;
				APPEND_CHAR(bufp, bufend, *cp++);
				while (isdigit((unsigned char) *cp) ||
					   (*cp == ':') || (*cp == '.'))
					APPEND_CHAR(bufp, bufend, *cp++);
			}
			/* date field? allow embedded text month */
			else if (*cp == '-' || *cp == '/' || *cp == '.')
			{
				/* save delimiting character to use later */
				char		delim = *cp;

				APPEND_CHAR(bufp, bufend, *cp++);
				/* second field is all digits? then no embedded text month */
				if (isdigit((unsigned char) *cp))
				{
					ftype[nf] = ((delim == '.') ? DTK_NUMBER : DTK_DATE);
					while (isdigit((unsigned char) *cp))
						APPEND_CHAR(bufp, bufend, *cp++);

					/*
					 * insist that the delimiters match to get a three-field
					 * date.
					 */
					if (*cp == delim)
					{
						ftype[nf] = DTK_DATE;
						APPEND_CHAR(bufp, bufend, *cp++);
						while (isdigit((unsigned char) *cp) || *cp == delim)
							APPEND_CHAR(bufp, bufend, *cp++);
					}
				}
				else
				{
					ftype[nf] = DTK_DATE;
					while (isalnum((unsigned char) *cp) || *cp == delim)
						APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				}
			}

			/*
			 * otherwise, number only and will determine year, month, day, or
			 * concatenated fields later...
			 */
			else
				ftype[nf] = DTK_NUMBER;
		}
		/* Leading decimal point? Then fractional seconds... */
		else if (*cp == '.')
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			while (isdigit((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, *cp++);

			ftype[nf] = DTK_NUMBER;
		}

		/*
		 * text? then date string, month, day of week, special, or timezone
		 */
		else if (isalpha((unsigned char) *cp))
		{
			bool		is_date;

			ftype[nf] = DTK_STRING;
			APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
			while (isalpha((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));

			/*
			 * Dates can have embedded '-', '/', or '.' separators.  It could
			 * also be a timezone name containing embedded '/', '+', '-', '_',
			 * or ':' (but '_' or ':' can't be the first punctuation). If the
			 * next character is a digit or '+', we need to check whether what
			 * we have so far is a recognized non-timezone keyword --- if so,
			 * don't believe that this is the start of a timezone.
			 */
			is_date = false;
			if (*cp == '-' || *cp == '/' || *cp == '.')
				is_date = true;
			else if (*cp == '+' || isdigit((unsigned char) *cp))
			{
				*bufp = '\0';	/* null-terminate current field value */
				/* we need search only the core token table, not TZ names */
				if (datebsearch(field[nf], datetktbl, szdatetktbl) == NULL)
					is_date = true;
			}
			if (is_date)
			{
				ftype[nf] = DTK_DATE;
				do
				{
					APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				} while (*cp == '+' || *cp == '-' ||
						 *cp == '/' || *cp == '_' ||
						 *cp == '.' || *cp == ':' ||
						 isalnum((unsigned char) *cp));
			}
		}
		/* sign? then special or numeric timezone */
		else if (*cp == '+' || *cp == '-')
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			/* soak up leading whitespace */
			while (isspace((unsigned char) *cp))
				cp++;
			/* numeric timezone? */
			/* note that "DTK_TZ" could also be a signed float or yyyy-mm */
			if (isdigit((unsigned char) *cp))
			{
				ftype[nf] = DTK_TZ;
				APPEND_CHAR(bufp, bufend, *cp++);
				while (isdigit((unsigned char) *cp) ||
					   *cp == ':' || *cp == '.' || *cp == '-')
					APPEND_CHAR(bufp, bufend, *cp++);
			}
			/* special? */
			else if (isalpha((unsigned char) *cp))
			{
				ftype[nf] = DTK_SPECIAL;
				APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				while (isalpha((unsigned char) *cp))
					APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
			}
			/* otherwise something wrong... */
			else
				return DTERR_BAD_FORMAT;
		}
		/* ignore other punctuation but use as delimiter */
		else if (ispunct((unsigned char) *cp))
		{
			cp++;
			continue;
		}
		/* otherwise, something is not right... */
		else
			return DTERR_BAD_FORMAT;

		/* force in a delimiter after each field */
		*bufp++ = '\0';
		nf++;
	}

	*numfields = nf;

	return 0;
}


