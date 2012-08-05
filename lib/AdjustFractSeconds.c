

#include "datizo.h"

/*
 * Multiply frac by scale (to produce seconds) and add to *tm & *fsec.
 * We assume the input frac is less than 1 so overflow is not an issue.
 */
void
AdjustFractSeconds(double frac, struct tm * tm, fsec_t *fsec, int scale)
{
	int			sec;

	if (frac == 0)
		return;
	frac *= scale;
	sec = (int) frac;
	tm->tm_sec += sec;
	frac -= sec;
#ifdef HAVE_INT64_TIMESTAMP
	*fsec += rint(frac * 1000000);
#else
	*fsec += frac;
#endif
}


