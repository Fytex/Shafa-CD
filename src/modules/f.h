#ifndef MODULE_F_H
#define MODULE_F_H

#include <stdbool.h>

/**
\brief Compresses file with RLE's algorithm and saves it to disk
 @param path Original file's path
 @param force_rle Force execution of RLE's algorithm even if % of compression <= 5%
 @param block_size Size of each block
 @returns Success
*/
bool rle_compress(char * path, bool force_rle, int block_size);


/**
\brief Creates a table of frequencies and saves it to disk
 @param path Original/RLE file's path
 @param block_size Size of each block
 @returns Success
*/
bool get_frequencies(char * path, int block_size);

#endif //MODULE_F_H