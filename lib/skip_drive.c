
#include "datizo.h"

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
#endif

