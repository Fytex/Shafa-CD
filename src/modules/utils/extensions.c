#include <string.h>

#include "extensions.h"

bool check_extension(const char * const path, const char * const ext)
{
    char * path_ext = strrchr(path, '.');
    return path_ext && !strcmp(path_ext, ext);
}


void append_extension(char * const path, const char * const ext)
{
}


void remove_extension(char * const path, const char * const ext)
{
}