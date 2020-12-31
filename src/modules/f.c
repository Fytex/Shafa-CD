/***************************************************
 *
 *  Author(s): Ana Teixeira, João Carvalho
 *  Created Date: 3 Dec 2020
 *  Updated Date: 31 Dec 2020
 *
 **************************************************/

#include "f.h"
#include "utils/errors.h"
#include "utils/file.h"
#include "utils/extensions.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int block_compression(uint8_t buffer[], uint8_t block[], const int block_size, int size_f)
{
    //Looping variables(i,j,k,l), number of symbols int the beginning(i) and if compressed(comp_block_size)
    int i, j, size_block_rle;
    //Cycle that goes through the first block of symbols of the file
    for(i = 0, size_block_rle=0; i < block_size && i < size_f; i = j) {
        int n_reps = 0; //Number of repetitions of a symbol
        for(j = i; buffer[i] == buffer[j] && n_reps <255; ++j, ++n_reps); //Counts the number of repetitions of a symbol
        if(n_reps >= 4 || !buffer[i]) //If a symbol repits itself 4 times or more or if the symbol is null
        {
            block[size_block_rle] = 0;
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


//Function that turns block of compressed content(or not) in an array of frequencies(each index matches a symbol, from 0 to 255)
void make_freq(unsigned char* block, int* freq, int size_block)
{
    int i;
    for(i = 0; i < 256; i++) freq[i] = 0; //Puts all the elements of freq as 0
    for(i = 0; i < size_block; i++)  //Goes through the block
    {
        int symbol;
        symbol = block[i]; //Saves symbol
        ++freq[symbol]; //Increments frequency of a symbol
    
    }
}

_modules_error write_freq(int *freq, FILE* f_freq,  int block_num, int n_blocks) {
    int i, j, print = 0, print2 = 0, print3 = 0;
    _modules_error error = _SUCCESS;
    
    for(i = 0; i < 256;) //Goes through the block of frequencies
    {
        print = fprintf(f_freq,"%d", freq[i]); //writes the frequency of each value on the freq file
        if(print >= 1) {
            for(j = i; freq[i] == freq[j] && j<256 ; j++) //If the frequencies of consecutive values are the same writes ';' after the fisrt value
            {
                if(j!=255) {
                print2 = fprintf(f_freq, ";");
                if(print2 < 0) error = _FILE_STREAM_FAILED;
                }  
            }
            i = j;
        }
        else error = _FILE_STREAM_FAILED; 
    }
    if(block_num == n_blocks -1) {
        print3 = fprintf(f_freq, "@0");
        if(print3 < 1) error = _FILE_STREAM_FAILED;
    }

    return error;
}

static inline void print_summary(long long n_blocks, unsigned long *block_sizes, unsigned long *block_rle_sizes, float compression_ratio, double total_t, const char * const path_rle, const char * const path_freq, const char * const path_rle_freq) {

    printf(
        "Ana Rita Teixeira, a93276, MIEI/CD, 1-jan-2021\n"
        "João Carvalho, a93166, MIEI/CD, 1-jan-2021\n"
        "Module: f (calculation of symbol frequencies)\n"
        "Number of blocks: %lld\n" , n_blocks
    );
    
    //cycle to print the block sizes of the txt file
    printf("Size of blocks analyzed in the original file: ");
    for(long long i = 0; i < n_blocks; i++) {
        if(i == n_blocks - 1)
            printf("%lu\n", block_sizes[i]);
        else printf("%lu/", block_sizes[i]);
    }
    
    printf("RLE Compression: %s (%f%% compression)\n", path_rle, compression_ratio);
        
    //ciclo para tamamhos dos blocos do rle
    printf("Size of blocks analyzed in the RLE file: ");
    for(long long i = 0; i < n_blocks; i++) {
        if(i == n_blocks - 1)
            printf("%lu bytes\n", block_rle_sizes[i]);
        else printf("%lu/", block_rle_sizes[i]);
    }
    //printf("Size of blocks analyzed in the RLE file: %lu\n"); //dentro de um ciclo
    
    printf(
        "Module runtime (milliseconds): %f\n"
        "Generated files: %s, %s\n",
        total_t, path_freq, path_rle_freq
    );
}

_modules_error freq_rle_compress(char** const path, const bool force_rle, const bool force_freq, const unsigned long block_size)
{
    clock_t t; //NOVO
    double total_t;
    float compression_ratio;
    uint8_t *buffer, *block;
    int size_f, read, print;
    long long n_blocks;
    bool compress_rle;
    long size_of_last_block;
    char *path_rle, *path_rle_freq, *path_freq; 
    unsigned long the_block_size, size_block_rle, compresd, *block_sizes, *block_rle_sizes;
    FILE *f, *f_rle, *f_rle_freq, *f_freq;

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
            
            //Opening rle file
            f_rle = fopen(path_rle, "wb");
            if(f_rle) {
                
                //Creating path to freq file
                path_rle_freq = add_ext(path_rle, FREQ_EXT);
                if(path_rle_freq) {
                    
                    //Opening freq file
                    f_rle_freq = fopen(path_rle_freq, "wb");
                    if(f_rle_freq) {
                        
                        //Creating path to forced freq file
                        path_freq = add_ext(*path, FREQ_EXT);
                        if(path_freq) {
                            
                            //Opening forced freq file
                            f_freq = fopen(path_freq, "wb");
                            if(f_freq) {
                                
                                n_blocks = fsize(f, *path, &the_block_size, &size_of_last_block);
                                size_f = (n_blocks-1) * (int)the_block_size + (int)size_of_last_block; //Saves the size of the file
                                if(size_f >= _1KiB){
                                    
                                    buffer = malloc(size_f); //Allocates memory for all the characters in the file
                                    if(buffer){
                                        
                                        read = fread(buffer, 1, size_f, f); //Loads the content of the file into the buffer
                                        if(read == size_f){
                                            
                                            // divides and compresses the buffer into blocks and writes it into the rle file
                                            compresd = the_block_size;
                                            size_block_rle = 0;
                                            block_sizes = malloc(n_blocks * sizeof(unsigned long));
                                            if(block_sizes) {
                                                block_rle_sizes = malloc(n_blocks * sizeof(unsigned long));
                                                if(block_rle_sizes) {
                                                    for (int block_num = 0, s = 0; block_num < n_blocks; ++block_num) {
                                                        if(block_num == n_blocks -1) {
                                                            compresd = size_f - s;
                                                    
                                                        }
                                                        block_sizes[block_num] = compresd;
                                                        block = malloc(compresd * 3); //creates string with enough memory to compress the block
                                                        if(block) {
                                                    
                                                            size_block_rle = block_compression(buffer+s, block, compresd, size_f);
                                                            if(block_num == 0) {
                                                                compression_ratio = (float)(compresd - size_block_rle)/compresd; //Calculates the compression ratio
                                                                if(compression_ratio < 0.05 && !force_rle) compress_rle = false;
                                                            }
                                                            
                                                    

                                                            if(block_num == 0 && compress_rle) {
                                                                print = fprintf(f_rle_freq,"@R@%lld", n_blocks); //The start of the freq file: @R@n_blocks@size_block_freq@
                                                            }
                                                            if(block_num == 0 && (!compress_rle || force_freq)) {
                                                                print = fprintf(f_freq,"@N@%lld", n_blocks); //The start of the freq file: @R@n_blocks@size_block_freq@
                                                            }
                                                            
                                                            if(print >= 4) {

                                                                int *freq = malloc(sizeof(int)*256); //Allocates memory for all the symbol's frequencies
                                                                if(freq) {
                                                        
                                                                    if(compress_rle) {
                                                                        block_rle_sizes[block_num] = size_block_rle;
                                                                        int res = fwrite(block, 1, size_block_rle, f_rle); //Writes the first block of frquencies in the freq file
                                                                        if(res == size_block_rle){
                                                                            make_freq(block, freq, size_block_rle); //Generates an array of frequencies of the block
                                                                
                                                                            if(fprintf(f_rle_freq, "@%ld@", size_block_rle) >= 2) {
                                                                                error = write_freq(freq, f_rle_freq, block_num, n_blocks);
                                                                            }
                                                                            else error = _FILE_STREAM_FAILED;
                                                                
                                                                        }
                                                                        else error = _FILE_STREAM_FAILED;
                                                                    }
                                                                    if(!compress_rle || force_freq) {
                                                            
                                                                        make_freq(buffer+s, freq, compresd); //Generates an array of frequencies of the block
                                                            
                                                                        if(fprintf(f_freq, "@%ld@", compresd) >= 2) {
                                                                            error = write_freq(freq, f_freq, block_num, n_blocks);
                                                                            
                                                                        }
                                                                        else error = _FILE_STREAM_FAILED;
                                                            
                                                                    }
                                                                    free(freq);
                                                    
                                                                }
                                                                else error = _LACK_OF_MEMORY;
                                                            }
                                                            else error = _FILE_STREAM_FAILED;

                                                        s+=compresd;
                                                        free(block); 
                                                        }
                                                    else error = _LACK_OF_MEMORY; 
                                                    }
                                                

                                                }
                                                else error = _LACK_OF_MEMORY;

                                            
                                            }
                                            else error = _LACK_OF_MEMORY;
                                        }
                                        else error = _FILE_STREAM_FAILED;
                                        free(buffer); 
                                    }
                                    else error = _LACK_OF_MEMORY;   
                                }
                                else error = _FILE_TOO_SMALL; //If the file is too small
                                fclose(f_freq);
                            }
                            else error = _FILE_INACCESSIBLE;
                            free(path_freq);
                        }
                        else error = _LACK_OF_MEMORY;

                        fclose(f_rle_freq);
                    }
                    else error = _FILE_INACCESSIBLE;
                    free(path_rle_freq);
                }
                else error = _LACK_OF_MEMORY;

                fclose(f_rle);
            }
            else error = _FILE_INACCESSIBLE;
            //if(!error) {
                //free(*path);
                //*path = path_rle;
            //free(path_rle);
            //}
        }
        else error = _LACK_OF_MEMORY;
        
        fclose(f);

    }
    else error = _FILE_INACCESSIBLE;
    
    if(!error){
        t = clock() - t;
        total_t = (((double) t) / CLOCKS_PER_SEC) * 1000;
        print_summary(n_blocks, block_sizes, block_rle_sizes, compression_ratio, total_t, path_rle,  path_freq, path_rle_freq);
        free(block_sizes);
        free(block_rle_sizes);
    }

    return error;
}

