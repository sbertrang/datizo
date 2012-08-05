
#include "datizo.h"

/* dateimtime.c */
const int	day_tab[2][13] =
{
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0}
};

int                        DateOrder = DATEORDER_MDY;

