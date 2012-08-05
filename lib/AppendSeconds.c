
#include <stdio.h>
#include <stdlib.h>

#include "datizo.h"

/*
 * Append sections and fractional seconds (if any) at *cp.
 * precision is the max number of fraction digits, fillzeros says to
 * pad to two integral-seconds digits.
 * Note that any sign is stripped from the input seconds values.
 */
void
AppendSeconds(char *cp, int sec, fsec_t fsec, int precision, bool fillzeros)
{
	if (fsec == 0)
	{
		if (fillzeros)
			sprintf(cp, "%02d", abs(sec));
		else
			sprintf(cp, "%d", abs(sec));
	}
	else
	{
#ifdef HAVE_INT64_TIMESTAMP
		if (fillzeros)
			sprintf(cp, "%02d.%0*d", abs(sec), precision, (int) Abs(fsec));
		else
			sprintf(cp, "%d.%0*d", abs(sec), precision, (int) Abs(fsec));
#else
		if (fillzeros)
			sprintf(cp, "%0*.*f", precision + 3, precision, fabs(sec + fsec));
		else
			sprintf(cp, "%.*f", precision, fabs(sec + fsec));
#endif
		TrimTrailingZeros(cp);
	}
}


