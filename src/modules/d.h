#ifndef MODULE_D_H
#define MODULE_D_H

#include "utils/errors.h"

/**
\brief Loads a block of a RLE file into a string
 @param f_rle RLE file that will be saved in the string
 @param block_size Size of the block that is going to be loaded
 @returns Error status
*/
char* loadingInfo (FILE* f_rle, int block_size);

/**
\brief Decompresses from the RLE format to the original one
 @param buffer String that contains a block of the RLE file
 @param block_size Size of the block that is going to be decompressed
 @param size_sequence Adress in which to load the size of the decompressed string
 @returns Error status
*/
char* decompress_string (char* buffer, int block_size, int* size_sequence);

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
