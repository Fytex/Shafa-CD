#ifndef MODULE_D_H
#define MODULE_D_H

#include <stdbool.h>

#include "utils/errors.h"

typedef struct {
    unsigned long * sizes;
    long long length;
} BlocksSize;


/**
\brief Decompresses file which was compressed with Shannon Fano's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @param decompress_rle Decompresses file with RLE's algorithm too
 @returns Error status
*/
_modules_error _shafa_decompress(char ** path, bool decompress_rle);


/**
\brief Decompresses file which was compressed with RLE's algorithm and saves it to disk.
This is an internal function which can be used in specific cases where the programmer already knows the size of each block from Original's file.
 @param path Pointer to the original/RLE file's path
 @param blocks_size Blocks Size struct
 @returns Error status
*/
_modules_error _rle_decompress(char ** path, const BlocksSize blocks_size);


/**
\brief Decompresses file which was compressed with RLE's algorithm and saves it to disk.
 @param path Pointer to the original/RLE file's path
 @returns Error status
*/
static inline _modules_error rle_compress(char ** path)
{
    return _rle_compress(path, (BlocksSize) {NULL, 0});
}


#endif //MODULE_D_H
