/* postgresql: src/backend/utils/adt/datetime.c */

#include <string.h>

#include "datizo.h"

/* TrimTrailingZeros()
 * ... resulting from printing numbers with full precision.
 *
 * Before Postgres 8.4, this always left at least 2 fractional digits,
 * but conversations on the lists suggest this isn't desired
 * since showing '0.10' is misleading with values of precision(1).
 */
void
TrimTrailingZeros(char *str)
{
	int			len = strlen(str);

	while (len > 1 && *(str + len - 1) == '0' && *(str + len - 2) != '.')
	{
		len--;
		*(str + len) = '\0';
	}
}


