/***************************************************
 *
 *  Author(s): Ana Teixeira, João Carvalho
 *  Created Date: 3 Dec 2020
 *  Updated Date: 3 Jan 2021
 *
 **************************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


#include "utils/file.h"
#include "utils/errors.h"
#include "utils/extensions.h"

/**
\brief Compresses a block
 @param buffer Array loaded with the original file content
 @param block Array where to load the compressed content
 @param block_size Size of the current block
 @param size_f Size of the original file
 @returns Size of the compressed block
*/
static unsigned long block_compression(const uint8_t buffer[], uint8_t block[], const unsigned long block_size, unsigned long size_f)
{
    //Looping variables(i,j)
    unsigned long i, j, size_block_rle;
    //Cycle that goes through the block of symbols of the file
    for(i = 0, size_block_rle=0; i < block_size && i < size_f; i = j) {
        //Number of repetitions of a symbol
        int n_reps = 0;
        //Counts the number of repetitions of a symbol
        for(j = i; buffer[i] == buffer[j] && n_reps <255 && j<block_size; ++j, ++n_reps);
        //If a symbol repits itself 4 times or more or if the symbol is NULL
        if(n_reps >= 4 || !buffer[i])
        {
            block[size_block_rle] = 0;
            block[size_block_rle+1] = buffer[i];
            block[size_block_rle+2] = n_reps;
            size_block_rle +=3;
        }
        else
        {
            block[size_block_rle] = buffer[i];
            size_block_rle++;
            j = ++i; 
        } 
    }
    return size_block_rle;
}

/**
\brief Turns block of content in an array of frequencies (each index matches a symbol from 0 to 255)
 @param block Array with the symbols (current block)
 @param freq Array to put the frequencies
 @param size_block Block size
*/
static void make_freq(const unsigned char* block, unsigned long* freq, unsigned long size_block)
{
    int i;
    unsigned long j;
    //Puts all the elements of freq as 0
    for(i = 0; i < 256; i++) freq[i] = 0;
    //Goes through the block
    for(j = 0; j < size_block; j++)
    {
        int symbol;
        //Saves symbol
        symbol = block[j];
        //Increments frequency of the symbol
        ++freq[symbol];
    
    }
}

/**
\brief Writes the frequencies in the freq file
 @param freq Array with the frequencies
 @param f_freq Freq file where we load the content
 @param block_num Current block
 @param n_blocks Number of blocks
 @returns Error status
*/
static _modules_error write_freq(const unsigned long *freq, FILE* f_freq, const unsigned long long block_num, const unsigned long long n_blocks) 
{
    int i, j, print = 0, print2 = 0, print3 = 0;
    _modules_error error = _SUCCESS;
    //Goes through the block of frequencies
    for(i = 0; i < 256;)
    {   //Writes the frequency of each value in the freq file
        print = fprintf(f_freq,"%lu", freq[i]);
        //Verifys if the fprintf went well
        if(print >= 1) {
            //If the frequencies of consecutive values are the same writes ';' after the fisrt value
            for(j = i; freq[i] == freq[j] && j<256 ; j++)
            {
                if(j!=255) {
                    print2 = fprintf(f_freq, ";");
                    //If the fprintf went wrong
                    if(print2 < 0) error = _FILE_STREAM_FAILED;
                }  
            }
            i = j;
        }
        else error = _FILE_STREAM_FAILED; 
    }//If it's the last block
    if(block_num == n_blocks -1) {
        print3 = fprintf(f_freq, "@0");
        //If the fprintf went wrong
        if(print3 < 1) error = _FILE_STREAM_FAILED;
    }

    return error;
}

/**
\brief Prints the results of the program execution
 @param n_blocks Number of blocks
 @param block_sizes Block sizes of the original file
 @param size_f size of the original file
 @param block_rle_sizes Block sizes of the rle file
 @param total_t Time that the program took to execute
 @param path_rle Path to the rle file
 @param path_freq Path to the freq file from the txt file
 @param path_rle_freq Path to the freq file from the rle file
*/
static inline void print_summary(unsigned long long n_blocks, unsigned long *block_sizes, unsigned long size_f, unsigned long *block_rle_sizes, double total_t, const char * const path_rle, const char * const path_freq, const char * const path_rle_freq) 
{
    printf(
        "Ana Rita Teixeira, a93276, MIEI/CD, 1-jan-2021\n"
        "João Carvalho, a93166, MIEI/CD, 1-jan-2021\n"
        "Module: f (calculation of symbol frequencies)\n"
        "Number of blocks: %lu\n" , n_blocks
    );
    
    printf("Size of blocks analyzed in the original file: ");
    //Cycle to print the block sizes of the txt file
    for(unsigned long long i = 0; i < n_blocks; i++) {
        if(i == n_blocks - 1)
            printf("%lu\n", block_sizes[i]);
        else printf("%lu/", block_sizes[i]);
    }
    
    if(path_rle) {
        unsigned long size_rle = 0;
        long compression;
        float compression_ratio;
        for(unsigned long long j = 0; j<n_blocks; j++) size_rle += block_rle_sizes[j];
        compression = size_f - size_rle;
        compression_ratio = (float)compression/(float)size_f; 
        compression_ratio*=100.0;
        printf("RLE Compression: %s (%f%% compression)\n", path_rle, compression_ratio);
        
        printf("Size of blocks analyzed in the RLE file: ");
        //Cycle to print the block sizes of the rle file
    
        for(unsigned long long i = 0; i < n_blocks; i++) {
            if(i == n_blocks - 1)
                printf("%lu bytes\n", block_rle_sizes[i]);
            else printf("%lu/", block_rle_sizes[i]);
        }
    }
    printf("Module runtime (milliseconds): %f\n", total_t);
    printf("Generated files: ");

    if(path_freq && path_rle_freq)
        printf("%s, %s\n", path_freq, path_rle_freq);
    else if(path_freq)
        printf("%s\n", path_freq);
    else if(path_rle_freq)
        printf("%s\n", path_rle_freq);
}


_modules_error freq_rle_compress(char** const path, const bool force_rle, const bool force_freq, const unsigned long block_size)
{
    clock_t t; 
    float total_t;
    float compression_ratio;
    uint8_t *buffer, *block;
    int  print_rle, print;
    long compression;
    unsigned long long n_blocks, block_num;
    bool compress_rle;
    long size_of_last_block;
    char *path_rle = NULL, *path_rle_freq = NULL, *path_freq = NULL; 
    unsigned long size_f, the_block_size, size_block_rle, compresd, *block_sizes, *block_rle_sizes, s;
    FILE *f, *f_rle=NULL, *f_rle_freq=NULL, *f_freq=NULL;

    compress_rle = true;
    size_of_last_block = 0;
    the_block_size = block_size;
    _modules_error error = _SUCCESS;

    t = clock();

    //Opening txt file
    f = fopen(*path, "rb");
    if(f){
        
        //Creating path to rle file
        path_rle = add_ext(*path, RLE_EXT);
        if(path_rle){
            //Creating path to freq file
            path_rle_freq = add_ext(path_rle, FREQ_EXT);
            if(path_rle_freq) { 
                //Creating path to forced freq file
                path_freq = add_ext(*path, FREQ_EXT);
                if(path_freq) {
                    //Getting number of blocks of the txt file
                    n_blocks = fsize(f, *path, &the_block_size, &size_of_last_block);
                    //Getting the size of the txt file
                    size_f = (n_blocks-1) * (int)the_block_size + (int)size_of_last_block;
                    //If txt file size is at least 1KiB
                    if(size_f >= _1KiB){        
                                    
                        compresd = the_block_size;
                        size_block_rle = 0;
                        //Allocates memory for the array that will contain the block sizes of the txt file
                        block_sizes = malloc(n_blocks * sizeof(unsigned long));
                        if(block_sizes) {
                            //Allocates memory for the array that will contain the block sizes of the rle file
                            block_rle_sizes = malloc(n_blocks * sizeof(unsigned long));
                            if(block_rle_sizes) {
                                //Divides the buffer into blocks
                                for (block_num = 0, s = 0; block_num < n_blocks; ++block_num) {
                                    //If it's the last block
                                    if(block_num == n_blocks -1) {
                                        compresd = size_f - s;                                            
                                    }
                                    //Loads size of the current block of the txt file to the respective array
                                    block_sizes[block_num] = compresd;
                                    //Allocates memory for the array that will contain the content of the txt file
                                    buffer = malloc(compresd * sizeof(uint8_t));
                                    if(buffer) {
                                        //Loads the content of the block of the txt file into the buffer
                                        if(fread(buffer, sizeof(uint8_t), compresd, f) == compresd) {
                                            //Allocates memory for the array that will contain the compressed content of the buffer
                                            block = malloc(compresd * 2 + 3); // (size/2 + 1) * 3 + size/2 = 2*size + 3
                                            if(block) {
                                                if(compress_rle) {
                                                    //Compresses the current block and returns its size
                                                    size_block_rle = block_compression(buffer, block, compresd, size_f);
                                                    //If it's the first block
                                                    if(block_num == 0) {
                                                        //Calculates the compression rate
                                                        compression = compresd - size_block_rle;
                                                        compression_ratio = (float)compression/(float)compresd;
                                                                    
                                                                    
                                                        //If the rate is lower than 5% and the user didn't force the rle file
                                                        if(compression_ratio < 0.05 && !force_rle) compress_rle = false;
                                                    }
                                                }

                                                //Opening rle file
                                                if(!f_rle && compress_rle) {
                                                    f_rle = fopen(path_rle, "wb");
                                                    if(!f_rle) {
                                                        error = _FILE_INACCESSIBLE;
                                                        break;
                                                    }
                                                }
                                                //Opening rle freq file
                                                if(!f_rle_freq && compress_rle) {
                                                    f_rle_freq = fopen(path_rle_freq, "wb");
                                                    if(!f_rle_freq) {
                                                        error = _FILE_INACCESSIBLE;
                                                        break;
                                                    }
                                                }
                                                //Opening freq file
                                                if(!f_freq && (force_freq || (!compress_rle))) {
                                                f_freq = fopen(path_freq, "wb");
                                                    if(!f_freq) {
                                                        error = _FILE_INACCESSIBLE;
                                                        break;
                                                    }
                                                }
                                                                        
                                                //If it's the first block and the user forced the rle file
                                                if(block_num == 0 && compress_rle) {
                                                    //Prints the header of the freq file: @R@n_blocks
                                                    print_rle = fprintf(f_rle_freq,"@R@%lu", n_blocks);
                                                }
                                                //If it's the first block and the user didn't forced the rle file or forced the freq file
                                                if(block_num == 0 && (!compress_rle || force_freq)) {
                                                    //Prints the header of the freq file: @N@n_blocks
                                                    print = fprintf(f_freq,"@N@%lu", n_blocks);
                                                }
                                                //If the fprintf went well
                                                if((print >= 4 && print_rle >= 4) || (print >= 4 && !compress_rle) || print_rle >= 4) {
                                                    //Allocates memory for all the 256 symbol's frequencies
                                                    unsigned long *freq = malloc(sizeof(unsigned long)*256);
                                                    if(freq) {
                                                        //If it can be compressed
                                                        if(compress_rle) {
                                                                        
                                                            //Loads size of the current block of the rle file to the respective array
                                                            block_rle_sizes[block_num] = size_block_rle;
                                                            //Writes each compressed block in the rle file
                                                            int res = fwrite(block, 1, size_block_rle, f_rle);
                                                            if(res == size_block_rle){
                                                                //Generates an array of frequencies of the block (rle file content)
                                                                make_freq(block, freq, size_block_rle);
                                                                //Prints the size of the current compressed block in the freq file
                                                                if(fprintf(f_rle_freq, "@%lu@", size_block_rle) >= 2) {
                                                                    //Writes each frequencies block in the freq file from the rle file
                                                                    error = write_freq(freq, f_rle_freq, block_num, n_blocks);
                                                                }
                                                                else error = _FILE_STREAM_FAILED;
                                                        
                                                            }
                                                            else error = _FILE_STREAM_FAILED;
                                                        }
                                                        //If it can't be compressed or if the user forced the freq file
                                                        if(!compress_rle || force_freq) {
                                                                        
                                                            //Generates an array of frequencies of the block (txt file content)
                                                            make_freq(buffer, freq, compresd);
                                                            //Prints the current block size in the freq file
                                                            if(fprintf(f_freq, "@%lu@", compresd) >= 2) {
                                                                //Writes each frequencies block in the freq file from the txt file
                                                                error = write_freq(freq, f_freq, block_num, n_blocks);
                                                                            
                                                            }
                                                            else error = _FILE_STREAM_FAILED;
                                                            
                                                        }
                                                        free(freq);
                                                    
                                                    }
                                                    else error = _LACK_OF_MEMORY;
                                                }
                                                else error = _FILE_STREAM_FAILED;

                                            
                                                free(block);
                                            }
                                            else error = _LACK_OF_MEMORY;

                                                        
                                        }
                                        else error = _FILE_STREAM_FAILED;

                                        s+=compresd;
                                        free(buffer);
                                    }
                                    else error = _LACK_OF_MEMORY;
                                                
                                }
                                if(f_rle) fclose(f_rle);
                                if(f_freq) fclose(f_freq);
                                if(f_rle_freq) fclose(f_rle_freq);
                            }
                            else error = _LACK_OF_MEMORY;
                                    
                        }
                        else error = _LACK_OF_MEMORY;  
                    }
                    else error = _FILE_TOO_SMALL; //If the file is too small
                            
                    if(error || (!force_freq && (force_rle || compress_rle))){
                        free(path_freq);
                        path_freq = NULL;
                    }
                }
                else error = _LACK_OF_MEMORY;
            
                if(error || (!force_rle && !compress_rle)){
                    free(path_rle_freq);
                    path_rle_freq = NULL;
                }
            }
            else error = _LACK_OF_MEMORY;

            
            if(error || (!compress_rle && !force_rle)){
                free(path_rle);
                path_rle = NULL;
            }
        }
        else error = _LACK_OF_MEMORY;
        
        fclose(f);

    }
    else error = _FILE_INACCESSIBLE;
    //If there was no error
    if(!error){
        if (compress_rle || force_rle) {
            free(*path);
            *path = path_rle;
        }
        //Calculates the runtime
        t = clock() - t;
        //Calculates the time in milliseconds
        total_t = (float) ((((double) t) / CLOCKS_PER_SEC) * 1000);
        print_summary(n_blocks, block_sizes, size_f, block_rle_sizes, total_t, path_rle,  path_freq, path_rle_freq);
        free(block_sizes);
        free(block_rle_sizes);
        if(path_freq) free(path_freq);
        if(path_rle_freq) free(path_rle_freq);
    }

    return error;
}
