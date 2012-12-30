/* postgresql: src/backend/utils/misc/guc.c */

#include "datizo.h"

/*
 */
void
assign_timezone_abbreviations(const char *newval, void *extra)
{
	/* Do nothing for the boot_val default of NULL */
	if (!extra)
		return;

	InstallTimeZoneAbbrevs((TimeZoneAbbrevTable *) extra);
}
