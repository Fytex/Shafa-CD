/***************************************************
 *
 *  Author(s): Ana Teixeira, João Carvalho
 *  Created Date: 3 Dec 2020
 *  Updated Date: 16 Dec 2020
 *
 **************************************************/

#include "f.h"
#include "errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int block_compression(char * buffer, char *block, const int block_size, int size_f)
{
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

algo make_freq_txt(char* buffer)
{
    if(f)
    {
        int* freq = malloc(sizeof(int)*256); //Allocates memory for all the symbol's frequencies
        for(int i = 0; i < 256; i++) freq[i] = 0; //Puts all the elements of freq_rle as 0
        for(int i = 0; buffer[i]; i++) //Goes through the buffer
        {
            int symbol = buffer[i]; //Saves symbol
            ++freq[symbol]; //Increments frequency of a symbol
            
        }
    }
}

//Function that turns block of compressed content(or not) in an array of frequencies(each index matches a symbol, from 0 to 255)
algo make_freq_rle(char* block)
{
    if(f_rle)
    {
        int* freq_rle = malloc(sizeof(int)*256); //Allocates memory for all the symbol's frequencies
        for(int i = 0; i < 256; i++) freq_rle[i] = 0; //Puts all the elements of freq_rle as 0
        for(int i = 0; block[i]; i++)  //Goes through the block
        {
            {
                int symbol;
                if(block[i] == NULL); //If it's a compressed string of equal symbols
                {
                    symbol = block[++i]; //Saves symbol
                    freq_rle[symbol] = block[++i]; //Increments frequency of a symbol

                }

                symbol = block[i]; //Saves symbol
                ++freq_rle[symbol]; //Increments frequency of a symbol
            }
        }
    }
}


_modules_error freq_rle_compress(char** const path, const bool force_rle, const int block_size)
{
    //Reading and loading file into the buffer
    char* file_name = *path;
    FILE* f = fopen(file_name, "rb");
    if(!f) return _FILE_INACCESSIBLE;
    

    fseek(f, 0, SEEK_END); //Goes to the end of the file
    int size_f = ftell(f); //Saves the size of the file
    rewind(f); //Goes to the begining of the file

    if(size_f < 1024 && !force_rle) return _FILE_TOO_SMALL; //If the file is too small

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

    // divides and compresses the buffer into blocks and writes it into the rle file
    int num_blocks=0, compresd, size_block_rle = 0;
    for(int s = 0; s < size_f; ++num_blocks) {
        if(size_f - s + block_size < 1024 || size_f - s < block_size) // if the block is shorter than the default block size or if the last block is smaller than 1KB 
        {
            compresd = size_f - s; // real block size
        }
        else {
            compresd = block_size;
        }
        block = malloc(sizeof(char) * compresd); //creates string with enough memory to compress the block
        size_block_rle = block_compression(buffer+s, block, compresd, size_f); 
        if(num_blocks == 0) {
            double compression_ratio = (compresd - size_block_rle)/compresd; //Calculates the compression ratio
            if(compression_ratio < 0.05 && !force_rle) return _FILE_TOO_SMALL;
        }
        fwrite(block, 1, size_block_rle, f_rle);
        //onde tratar das freq
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