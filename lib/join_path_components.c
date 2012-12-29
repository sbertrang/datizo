/* postgresql: src/port/path.c */

#include "datizo.h"

#include <sys/param.h>
#include <string.h>
#include <stdio.h>

#ifdef WIN32

static char *
skip_drive(const char *path)
{
        if (IS_DIR_SEP(path[0]) && IS_DIR_SEP(path[1]))
        {
                path += 2;
                while (*path && !IS_DIR_SEP(*path))
                        path++;
        }
        else if (isalpha((unsigned char) path[0]) && path[1] == ':')
        {
                path += 2;
        }
        return (char *) path;
}
#else

#define skip_drive(path)        (path)
#endif

/*
 */
void
join_path_components(char *ret_path, const char *head, const char *tail)
{
	if (ret_path != head)
		strlcpy(ret_path, head, MAXPATHLEN);

	/*
	 * Remove any leading "." in the tail component.
	 *
	 * Note: we used to try to remove ".." as well, but that's tricky to get
	 * right; now we just leave it to be done by canonicalize_path() later.
	 */
	while (tail[0] == '.' && IS_DIR_SEP(tail[1]))
		tail += 2;

	if (*tail)
	{
		/* only separate with slash if head wasn't empty */
		snprintf(ret_path + strlen(ret_path), MAXPATHLEN - strlen(ret_path),
				 "%s%s",
				 (*(skip_drive(head)) != '\0') ? "/" : "",
				 tail);
	}
}

