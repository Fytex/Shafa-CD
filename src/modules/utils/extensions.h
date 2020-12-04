#ifndef UTILS_EXTENSIONS_H
#define UTILS_EXTENSIONS_H

#include <stdbool.h>


/**
\brief Checks whether the file has the given extension
 @param path File's path
 @param ext Extension
 @returns Success
*/
bool check_extension(const char * path, const char * ext);


/**
\brief Appends an extension from a file's path
 @param path File's path
 @param ext Extension
 @returns Success
*/
bool append_extension(char * path, const char * ext);


/**
\brief Removes an extension from a file's path
 @param path File's path
 @param ext Extension
 @returns Success
*/
bool remove_extension(char * path, const char * ext);

#endif //UTILS_EXTENSIONS_H