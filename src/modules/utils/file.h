#ifndef UTILS_FILE_H
#define UTILS_FILE_H

#include <stdio.h>

enum {
    _1KiB   = 1024,
    _64KiB  = 65536,
    _640KiB = 655360,
    _8MiB   = 8388608,
    _64MiB  = 67108864
};


/**
\brief Calculate the whole file size per block. O.S. Independent
 @param fp_in File Descriptor
 @param the_block_size Pointer to the block size
 @param size_of_last_block Pointer to the size of the last block
 @returns Number of blocks which have `*the_block_size` bytes
*/
long long fsize(FILE *fp_in, unsigned char *filename, unsigned long *the_block_size, long *size_of_last_block);

#endif //UTILS_FILE_H