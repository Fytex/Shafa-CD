/************************************************
 *
 *  Author(s): Pedro Tavares
 *  Created Date: 4 Dec 2020
 *  Updated Date: 7 Dec 2020
 *
 ***********************************************/


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "extensions.h"

// If used strrchr the worst case would be a long path without a '.' because it would compare every letter
bool check_ext(const char * const path, const char * const ext)
{
    size_t diff;

    if (path) {
        diff = strlen(path) - strlen(ext);

        if (diff >= 0)
            return (!strcmp(path + diff, ext));
    }

    return false;
}

char * add_ext(const char * const path, const char * const ext)
{
    size_t len_path = strlen(path);
    size_t len_ext = strlen(ext);
    char * new_path = NULL;

    new_path = malloc(len_path + len_ext + 1);

    if (new_path) {
        memcpy(new_path, path, len_path);
        memcpy(new_path + len_path, ext, len_ext + 1);
    }

    return new_path;
}


char * rm_ext(const char * const path)
{
    size_t len_path_no_ext;
    char * path_ext = strrchr(path, '.');
    char * new_path = NULL;

    len_path_no_ext = path_ext ? path_ext - path: strlen(path); 

    new_path = malloc(len_path_no_ext + 1);

    if (new_path) {
        memcpy(new_path, path, len_path_no_ext);
        new_path[len_path_no_ext] = '\0';
    }

    return new_path;
}