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
\brief Allocates a new `string` copying file's path's content and appends an extension
 @param path File's path
 @param ext Extension
 @returns File's path or NULL if an error ocurred while allocating memory
*/
char * add_ext(const char * path, const char * ext);


/**
\brief Allocates a new `string` copying file's path's content and removes the extension
 @param path File's path
 @returns File's path or NULL if no extension or if allocation fails
*/
char * rm_ext(const char * path);

#endif //UTILS_EXTENSIONS_H