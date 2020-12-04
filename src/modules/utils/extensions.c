#include <string.h>

#include "extensions.h"

bool check_extension(const char * const path, const char * const ext)
{
    char * path_ext = strrchr(path, '.');
    return path_ext && !strcmp(path_ext, ext);
}


bool append_extension(char * const path, const char * const ext)
{
    return true;
}


bool remove_extension(char * const path, const char * const ext)
{
    return true;
}