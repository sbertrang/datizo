
#include <string.h>

#include "datizo.h"

/* DecodeUnits()
 * Decode text string using lookup table.
 * This routine supports time interval decoding
 * (hence, it need not recognize timezone names).
 */
int
DecodeUnits(int field, char *lowtoken, int *val)
{
	int			type;
	const datetkn *tp;

	tp = deltacache[field];
	if (tp == NULL || strncmp(lowtoken, tp->token, TOKMAXLEN) != 0)
	{
		tp = datebsearch(lowtoken, deltatktbl, szdeltatktbl);
	}
	if (tp == NULL)
	{
		type = UNKNOWN_FIELD;
		*val = 0;
	}
	else
	{
		deltacache[field] = tp;
		type = tp->type;
		if (type == TZ || type == DTZ)
			*val = FROMVAL(tp);
		else
			*val = tp->value;
	}

	return type;
}	/* DecodeUnits() */

