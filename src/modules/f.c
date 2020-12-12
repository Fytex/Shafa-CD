/***************************************************
 *
 *  Author(s): Ana Teixeira, João Carvalho
 *  Created Date: 3 Dec 2020
 *  Updated Date: 12 Dec 2020
 *
 **************************************************/

#include "f.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int block_compression(char * buffer, char *block, const int block_size, int size_f) {
    //Looping variables(i,j,k,l), number of symbols int the beginning(i) and if compressed(comp_block_size)
    int i, j, l, size_block_rle;
    //Cycle that goes through the first block of symbols of the file
    for(i = 0, size_block_rle=0; i < block_size && i < size_f; i = j) {
        int n_reps = 0; //Number of repetitions of a symbol
        for(j = i; buffer[i] == buffer[j]; ++j, ++n_reps); //Counts the number of repetitions of a symbol
        if(n_reps >= 4 || !buffer[i]) //If a symbol repits itself 4 times or more or if the symbol is null
        {
            block[size_block_rle] = NULL;
            block[size_block_rle+1] = buffer[i];
            block[size_block_rle+2] = n_reps;
            size_block_rle +=3;
        }
        else //If there were no repited symbols(nothing got compressed)
        {
            block[size_block_rle] = buffer[i];
            size_block_rle++;
            j = ++i; 
        } 
    }
    return size_block_rle;
}

_modules_error rle_compress(char** const path, const bool force_rle, const int block_size)
{
    //Reading and loading file into the buffer
    char* file_name = *path;
    FILE* f = fopen(file_name, "rb");
    if(!f) return _FILE_INACCESSIBLE;
    

    fseek(f, 0, SEEK_END); //Goes to the end of the file
    int size_f = ftell(f); //Saves the size of the file
    rewind(f); //Goes to the begining of the file

    if(size_f < 1024 && !force_rle) return _FILE_TOO_SMALL;

    char* buffer = malloc(sizeof(char) * size_f); //Allocates memory for all the characters in the file
    if(!buffer) return _LACK_OF_MEMORY;

    int read = fread(buffer, 1, size_f, f); //Loads the content of the file into the buffer
    if (read != size_f) return _FILE_CORRUPTED;

    fclose(f);

    //Writing in a new file
    FILE* f_rle = fopen("new.rle", "rb");
    if(!f_rle) return _FILE_INACCESSIBLE;

    char* block = malloc(sizeof(char) * block_size); //Allocates memory for the compressed(or not) content of the file
    if(!block) return _LACK_OF_MEMORY;

    // divides and compresses the buffer into blocks and writes it into nthe rle file
    int num_blocks=0, compresd, size_block_rle = 0;
    for(int s = 0; s < size_f; ++num_blocks) {
        if(size_f - s + block_size < 1024 || size_f - s < block_size) // if the block is shorter than the default block size or if the last block is smaller than 1KB 
        {
            compresd = size_f - s; // real block size
        }
        else {
            compresd = block_size;
        }
        if(num_blocks == 0) {
            double compression_ratio = (compresd - size_block_rle)/compresd; //Calculates the compression ratio
            if(compression_ratio < 0.05 && !force_rle) return _FILE_TOO_SMALL;
        }
        block = malloc(sizeof(char) * compresd); //creates string with enough memory to compress the block
        size_block_rle = block_compression(buffer+s, block, compresd, size_f); 
        fwrite(block, 1, size_block_rle, f_rle);
        free(block);
        s+=compresd;

    }

    fclose(f_rle);
    

    return _SUCCESS;
}


_modules_error get_frequencies(char * const path, const int block_size)
{
    return true;
}