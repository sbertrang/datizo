/* postgresql: src/backend/utils/adt/datetime.c */

#include "datizo.h"
#include <string.h>

bool
CheckDateTokenTable(const char *tablename, const datetkn *base, int nel)
{
	bool		ok = true;
	int			i;

	for (i = 1; i < nel; i++)
	{
		if (strncmp(base[i - 1].token, base[i].token, TOKMAXLEN) >= 0)
		{
			/* %.*s is safe since all our tokens are ASCII */
			warnx("ordering error in %s table: \"%.*s\" >= \"%.*s\"",
				 tablename,
				 TOKMAXLEN, base[i - 1].token,
				 TOKMAXLEN, base[i].token);
			ok = false;
		}
	}
	return ok;
}

