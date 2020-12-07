#ifndef MODULE_C_H
#define MODULE_C_H

#include <stdbool.h>

/**
\brief Compresses file with Shannon Fano's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @returns Success
*/
bool shafa_compress(char ** path);

#endif //MODULE_C_H