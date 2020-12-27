#ifndef MODULE_D_H
#define MODULE_D_H

#include <stdint.h>

#include "utils/errors.h"


/**
\brief Decompresses file which was compressed with RLE's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @param blocks_size Pointer to blocks size
 @returns Error status
*/
_modules_error rle_decompress(char ** path);


/**
\brief Decompresses file which was compressed with Shannon Fano's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @param blocks_size Pointer to blocks size
 @returns Error status
*/
_modules_error shafa_decompress(char ** path, bool rle_decompression);


#endif //MODULE_D_H
