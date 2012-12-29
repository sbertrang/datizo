/* postgresql: src/port/path.c */

#include "datizo.h"
#include <sys/param.h>
#include <string.h>

/*
 */

void
make_relative_path(char *ret_path, const char *target_path, const char *bin_path, const char *my_exec_path)
{
	int			prefix_len;
	int			tail_start;
	int			tail_len;
	int			i;

	/*
	 * Determine the common prefix --- note we require it to end on a
	 * directory separator, consider eg '/usr/lib' and '/usr/libexec'.
	 */
	prefix_len = 0;
	for (i = 0; target_path[i] && bin_path[i]; i++)
	{
		if (IS_DIR_SEP(target_path[i]) && IS_DIR_SEP(bin_path[i]))
			prefix_len = i + 1;
		else if (target_path[i] != bin_path[i])
			break;
	}
	if (prefix_len == 0)
		goto no_match;			/* no common prefix? */
	tail_len = strlen(bin_path) - prefix_len;

	/*
	 * Set up my_exec_path without the actual executable name, and
	 * canonicalize to simplify comparison to bin_path.
	 */
	strlcpy(ret_path, my_exec_path, MAXPATHLEN);
	trim_directory(ret_path);	/* remove my executable name */
	canonicalize_path(ret_path);

	/*
	 * Tail match?
	 */
	tail_start = (int) strlen(ret_path) - tail_len;
	if (tail_start > 0 &&
		IS_DIR_SEP(ret_path[tail_start - 1]) &&
		dir_strcmp(ret_path + tail_start, bin_path + prefix_len) == 0)
	{
		ret_path[tail_start] = '\0';
		trim_trailing_separator(ret_path);
		join_path_components(ret_path, ret_path, target_path + prefix_len);
		canonicalize_path(ret_path);
		return;
	}

no_match:
	strlcpy(ret_path, target_path, MAXPATHLEN);
	canonicalize_path(ret_path);
}

