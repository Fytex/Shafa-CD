/***************************************************
 *
 *  Author(s): Ana Teixeira, João Carvalho
 *  Created Date: 3 Dec 2020
 *  Updated Date: 16 Dec 2020
 *
 **************************************************/

#include "f.h"
#include "utils/errors.h"
#include "utils/file.h"
#include "utils/extensions.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void write_freq(int *freq, FILE* f_freq,  int block_num, int n_blocks) {
    int i, j;
    for(i = 0; i < 256;) //Goes through the block of frequencies
        {
            fprintf(f_freq,"%d", freq[i]); //writes the frequency of each value on the freq file
            for(j = i; freq[i] == freq[j] && j<256 ; j++) //If the frequency of consecutive values is the same writes ';' after the fisrt value
            {
                if(j!=255) { // provisorio
                    fprintf(f_freq, ";");
                }
                    
            }
        i = j;
    }
    if(block_num == n_blocks -1) {
        fprintf(f_freq, "@0");
    }
}

_modules_error freq_rle_compress(char** const path, const bool force_rle, const bool force_freq, const int block_size)
{
    //int* freq;
    //float compression_ratio;
    uint8_t *buffer, *block;
    int size_f, read, compresd, size_block_rle;
    long long n_blocks;
    bool compress_rle;
    long *size_of_last_block;
    char *path_rle, *path_rle_freq, *path_freq; 
    unsigned long *the_block_size;
    FILE *f, *f_rle, *f_rle_freq, *f_freq;

    compress_rle = true;
    size_of_last_block = 0;
    the_block_size = block_size;
    _modules_error error = _SUCCESS;

    //Opening txt file
    f = fopen(*path, "rb");
    if(f){
        printf("ok f\n");
        //Creating path to rle file
        path_rle = add_ext(*path, RLE_EXT);
        if(path_rle){
            printf("ok path_rle\n");
            //Opening rle file
            f_rle = fopen(path_rle, "wb");
            if(f_rle) {
                printf("ok f_rle\n");
                //Creating path to freq file
                path_rle_freq = add_ext(path_rle, FREQ_EXT);
                if(path_rle_freq) {
                    printf("ok path_rle_freq\n");
                    //Opening freq file
                    f_rle_freq = fopen(path_rle_freq, "wb");
                    if(f_rle_freq) {
                        printf("ok f_rle_freq\n");
                        //Creating path to forced freq file
                        path_freq = add_ext(*path, FREQ_EXT);
                        if(path_freq) {
                            printf("ok path_freq\n");
                            //Opening forced freq file
                            f_freq = fopen(path_freq, "wb");
                            if(f_freq) {
                                printf("ok f_freq\n");
                                n_blocks = fsize(f, *path, &the_block_size, &size_of_last_block);
                                size_f = (n_blocks-1) * (int)the_block_size + (int)size_of_last_block; //Saves the size of the file
                                if(size_f >= _1KiB){
                                    printf("ok size_f\n");
                                    buffer = malloc(size_f); //Allocates memory for all the characters in the file
                                    if(buffer){
                                        printf("ok buffer\n");
                                        read = fread(buffer, 1, size_f, f); //Loads the content of the file into the buffer
                                        if(read == size_f){
                                            printf("ok read\n");
                                            // divides and compresses the buffer into blocks and writes it into the rle file
                                            compresd = (int)the_block_size;
                                            size_block_rle = 0;
                                            for (int block_num = 0, s = 0; block_num < n_blocks; ++block_num) {
                                                if(block_num == n_blocks -1) {
                                                    compresd = size_f - s;
                                                }
                                                block = malloc(compresd * 3); //creates string with enough memory to compress the block
                                                if(block) {
                                                    printf("ok block\n");
                                                    size_block_rle = block_compression(buffer+s, block, compresd, size_f);
                                                    float compression_ratio = (float)(compresd - size_block_rle)/compresd; //Calculates the compression ratio
                                                    if(compression_ratio < 0.05 && !force_rle) compress_rle = false;

                                                    if(block_num == 0 && compress_rle) {
                                                        fprintf(f_rle_freq,"@R@%lld", n_blocks); //The start of the freq file: @R@n_blocks@size_block_freq@
                                                    }
                                                    if(block_num == 0 && (!compress_rle || force_freq)) {
                                                        fprintf(f_freq,"@N@%lld", n_blocks); //The start of the freq file: @R@n_blocks@size_block_freq@
                                                    }
                                                            
                                                    int *freq = malloc(sizeof(int)*256); //Allocates memory for all the symbol's frequencies
                                                    if(freq) {
                                                        printf("ok freq\n");
                                                        if(compress_rle) {
                                                            int res = fwrite(block, 1, size_block_rle, f_rle); //Writes the first block of frquencies in the freq file
                                                            if(res == size_block_rle){
                                                                make_freq(block, freq, size_block_rle); //Generates an array of frequencies of the block
                                                                printf("ok make_freq\n");
                                                                fprintf(f_rle_freq, "@%d@", size_block_rle);
                                                                write_freq(freq, f_rle_freq, block_num, n_blocks);
                                                                printf("ok writefreq\n");
                                                            }
                                                            else error = _FILE_STREAM_FAILED;
                                                        }
                                                        if(!compress_rle || force_freq) {
                                                            printf("ok if\n");
                                                            make_freq(buffer+s, freq, compresd); //Generates an array of frequencies of the block
                                                            printf("ok make2\n");
                                                            fprintf(f_freq, "@%d@", compresd);
                                                            write_freq(freq, f_freq, block_num, n_blocks);
                                                            printf("ok write2\n");
                                                        }
                                                    free(freq);
                                                    printf("ok freefreq\n");
                                                    }
                                                    else error = _LACK_OF_MEMORY;

                                                s+=compresd;

                                                free(block); 
                                                printf("ok freeblock\n");
                                                }
                                                else error = _LACK_OF_MEMORY; 
                                            }
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
            free(path_rle);
            //}
        }
        else error = _LACK_OF_MEMORY;

        fclose(f);

    }
    else error = _FILE_INACCESSIBLE;
    
    return error;
}



_modules_error get_frequencies(char * const path, const int block_size)
{
    return true;
}