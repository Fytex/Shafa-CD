#ifndef MODULE_D_H
#define MODULE_D_H

#include <stdbool.h>

#include "utils/errors.h"

#define rle_decompress(path) _rle_decompress(path, NULL)


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
For simple goals use `rle_decompress(path)` macro. 
 @param path Pointer to the original/RLE file's path
 @param blocks_size Blocks Size struct
 @returns Error status
*/
_modules_error _rle_decompress(char ** path, const BlocksSize blocks_size);


#endif //MODULE_D_H
