/* postgresql: src/port/path.c */

#include "datizo.h"

/*
 */
void
get_share_path(const char *my_exec_path, char *ret_path)
{
	make_relative_path(ret_path, PGSHAREDIR, PGBINDIR, my_exec_path);
}

