
#include "datizo.h"

/* dateimtime.c */
const int	day_tab[2][13] =
{
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0}
};

int                        DateOrder = DATEORDER_MDY;


const int mon_lengths[2][MONSPERYEAR] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

const int year_lengths[2] = {
	DAYSPERNYEAR, DAYSPERLYEAR
};


char wildabbr[] = WILDABBR;
const char gmt[] = "GMT";

bool HasCTZSet = false;
int      CTimeZone = 0;

char	   *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};

char	   *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
"Thursday", "Friday", "Saturday", NULL};


int			DateStyle = USE_ISO_DATES;

int			IntervalStyle = INTSTYLE_POSTGRES;

