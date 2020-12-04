#ifndef MODULE_T_H
#define MODULE_T_H

#include <stdbool.h>

/**
\brief Creates a table of Shanon Fano's codes and saves it to disk
 @param path Original/RLE file's path
 @returns Success
*/
bool get_shafa_codes(char * path);

#endif //MODULE_T_H