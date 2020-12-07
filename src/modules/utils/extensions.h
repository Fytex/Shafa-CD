#ifndef UTILS_EXTENSIONS_H
#define UTILS_EXTENSIONS_H

#include <string.h>
#include <stdbool.h>

#define RLE_EXT ".rle"
#define FREQ_EXT ".freq"
#define CODES_EXT ".cod"
#define SHAFA_EXT ".shaf"


/**
\brief Checks whether the file has the given extension
 @param path File's path
 @param ext Extension
 @returns Success
*/
bool check_ext(const char * path, const char * ext);


/**
\brief Adds an extension to the file's path
 @param path File's path. If allocation fails then this argument won't be freed
 @param ext Extension
 @param clone Allocating new space for `path + ext`
 @returns File's path or NULL if an error ocurred while allocating memory
*/
char * add_ext(char * path, const char * ext, bool clone);


/**
\brief Removes the extension from file's path. This function doesn't free memory space it will only substitute first extension's character to a '\0'
 @param path File's path
 @returns File's path
*/
char * rm_ext(char * path);

#endif //UTILS_EXTENSIONS_H