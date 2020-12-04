#ifndef MODULE_F_H
#define MODULE_F_H

#include <stdbool.h>

/**
\brief Compresses file with RLE's algorithm and saves it to disk
 @param path Original file's path
 @param force_rle Force execution of RLE's algorithm even if % of compression <= 5%
 @returns Success
*/
bool rle_compress(char * path, bool force_rle);


/**
\brief Creates a table of frequencies and saves it to disk
 @param path Original/RLE file's path
 @returns Success
*/
bool get_frequencies(char * path);

#endif //MODULE_F_H