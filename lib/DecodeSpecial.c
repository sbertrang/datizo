
#include <string.h>

#include "datizo.h"

int
DecodeSpecial(int field, char *lowtoken, int *val)
{
	int			type;
	const datetkn *tp;

	tp = datecache[field];
	if (tp == NULL || strncmp(lowtoken, tp->token, TOKMAXLEN) != 0)
	{
		tp = datebsearch(lowtoken, timezonetktbl, sztimezonetktbl);
		if (tp == NULL)
			tp = datebsearch(lowtoken, datetktbl, szdatetktbl);
	}
	if (tp == NULL)
	{
		type = UNKNOWN_FIELD;
		*val = 0;
	}
	else
	{
		datecache[field] = tp;
		type = tp->type;
		switch (type)
		{
			case TZ:
			case DTZ:
			case DTZMOD:
				*val = FROMVAL(tp);
				break;

			default:
				*val = tp->value;
				break;
		}
	}

	return type;
}

