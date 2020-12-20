#ifndef MODULE_D_H
#define MODULE_D_H

#include "utils/errors.h"


/**
\brief Decompresses file which was compressed with RLE's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @returns Error status
*/
_modules_error rle_decompress(char ** path);


/**
\brief Decompresses file which was compressed with Shannon Fano's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @returns Error status
*/
_modules_error shafa_decompress(char ** path);


#endif //MODULE_D_H
