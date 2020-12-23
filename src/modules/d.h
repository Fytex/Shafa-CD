#ifndef MODULE_D_H
#define MODULE_D_H

#include <stdint.h>

#include "utils/errors.h"


typedef struct {
    uint32_t * sizes;
    long long length;
} BlocksSize;


/**
\brief Decompresses file which was compressed with RLE's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @param blocks_size Pointer to blocks size
 @returns Error status
*/
_modules_error rle_decompress(char ** path, BlocksSize * blocks_size);


/**
\brief Decompresses file which was compressed with Shannon Fano's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @param blocks_size Pointer to blocks size
 @returns Error status
*/
_modules_error shafa_decompress(char ** path, const BlocksSize * blocks_size);


#endif //MODULE_D_H
