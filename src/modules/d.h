#ifndef MODULE_D_H
#define MODULE_D_H

#include <stdbool.h>

#include "utils/errors.h"

/**
\brief Decompresses file which was compressed with Shannon Fano's algorithm and saves it to disk
 @param path Pointer to the SHAFA->RLE file's path
 @param decompress_rle Decompresses file with RLE's algorithm too
 @returns Error status
*/
_modules_error shafa_decompress(char ** path, bool decompress_rle);


/**
\brief Decompresses file which was compressed with RLE's algorithm and saves it to disk.
 @param path Pointer to the RLE->Original file's path
 @returns Error status
*/
_modules_error rle_decompress(char ** path);

#endif //MODULE_D_H
