#ifndef MODULE_F_H
#define MODULE_F_H

#include <stdbool.h>

#include "utils/errors.h"

/**
\brief Compresses file with RLE's algorithm if needed and creates the respective output frequencies' table. Finally saves it to disk
 @param path Pointer to the original file's path
 @param force_rle Force execution of RLE's algorithm even if % of compression <= 5%
 @param block_size Size of each block
 @returns Error status
*/
_modules_error freq_rle_compress(char ** path, bool force_rle, int block_size);

#endif //MODULE_F_H
