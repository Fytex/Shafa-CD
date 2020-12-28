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

int block_compression(u_int8_t buffer[], u_int8_t block[], const int block_size, int size_f)
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
        fclose(f_freq);
    }
}

//NOTA para Mosca: force_freq será para forçar a criar freq para o f_original mesmo que tb façamos para o f_rle(Depois eliminar isto)
_modules_error freq_rle_compress(char** const path, const bool force_rle, const bool force_freq, const int block_size)
{
    //Reading and loading file into the buffer
    FILE* f = fopen(*path, "rb");
    bool compress_rle = true;
    if(!f) return _FILE_INACCESSIBLE;
    unsigned long * the_block_size = block_size;
    long *size_of_last_block = 0;
    long long n_blocks = fsize(f, *path, &the_block_size, &size_of_last_block);

    int size_f = (n_blocks-1) * (int)the_block_size + (int)size_of_last_block; //Saves the size of the file
    if(size_f < 1024)
    {
        fclose(f);
        return _FILE_TOO_SMALL;  //If the file is too small
    }

    uint8_t* buffer = malloc(size_f); //Allocates memory for all the characters in the file
    if(!buffer) 
    {
        fclose(f);
        return _LACK_OF_MEMORY;
    }

    int read = fread(buffer, 1, size_f, f); //Loads the content of the file into the buffer
    if (read != size_f) return _FILE_STREAM_FAILED;

    fclose(f);

    
    FILE* f_rle, * f_freq,* f_rle_freq; 
    uint8_t* block;

    // divides and compresses the buffer into blocks and writes it into the rle file
    int compresd = (int)the_block_size, size_block_rle = 0, size_block_freq = 0;
    for (int block_num = 0, s = 0; block_num < n_blocks; ++block_num) 
    {
        if(block_num == n_blocks -1)
        {
            compresd = size_f - s;
        }
        block = malloc(compresd * 3); //creates string with enough memory to compress the block
        if(!block) 
        {
            free(buffer); //NOTA Mosca: penso estar certo mas confirma (dps apagar isto)
            //fclose(f_freq);
            fclose(f_rle);
            return _LACK_OF_MEMORY;
        }
        size_block_rle = block_compression(buffer+s, block, compresd, size_f);
        if(block_num == 0 )
        {
            float compression_ratio = (double)(compresd - size_block_rle)/compresd; //Calculates the compression ratio
            if(compression_ratio < 0.05 && !force_rle) compress_rle = false;
            /*f_freq = fopen("new.freq", "wb");
            if(!f_freq) 
            {
                free(buffer); //NOTA Mosca: penso estar certo mas confirma (dps apagar isto)
                free(block);
                fclose(f_rle);
                return _FILE_INACCESSIBLE;
            }
            */
            if(compress_rle) //If there was compression
            {
                f_rle = fopen("new.rle", "wb");
                    if(!f_rle) return _FILE_INACCESSIBLE;
                f_rle_freq = fopen("new.rle.freq", "wb");
                if(!f_rle_freq) {
                    free(buffer);
                    free(block);
                    fclose(f_rle);
                    return _FILE_INACCESSIBLE;
                }
                fprintf(f_rle_freq,"@R@%lld", n_blocks); //The start of the freq file: @R@n_blocks@size_block_freq@
                
            }
            if(!compress_rle || force_freq) //freq do original(FAZER DIREITO)
            {
                f_freq = fopen("new.freq", "wb");
                if(!f_freq) 
                {
                    free(buffer); //NOTA Mosca: penso estar certo mas confirma (dps apagar isto)
                    free(block);
                    if(f_rle_freq) fclose(f_rle_freq);
                    fclose(f_rle);
                    return _FILE_INACCESSIBLE;
                }
                fprintf(f_freq,"@N@%lld", n_blocks); //The start of the freq file: @R@n_blocks@size_block_freq@
            }
            
            
        }
        int* freq = malloc(sizeof(int)*256); //Allocates memory for all the symbol's frequencies
        if(compress_rle) //If there was compression
        {
            int res = fwrite(block, 1, size_block_rle, f_rle);
            //Writes the first block of frquencies in the freq file
            
            make_freq(block, freq, size_block_rle); //Generates an array of frequencies of the block
            
            fprintf(f_rle_freq, "@%d@", size_block_rle);
            write_freq(freq, f_rle_freq, block_num, n_blocks);
            
            
        }
        
        if(!compress_rle || force_freq) //freq do original(FAZER DIREITO)
        {
            make_freq(buffer+s, freq, compresd); //Generates an array of frequencies of the block
            
            fprintf(f_freq, "@%d@", compresd);
            write_freq(freq, f_freq, block_num, n_blocks);
            
        }
        
        free(block); //Frees an allocated block
        
        s+=compresd;
    }
    free(buffer); //NOTA para Mosca: este free pode não ser aqui!(depois tirar isto)
    //fclose(f_rle_freq);
    //fclose(f_freq);
    if (f_rle) fclose(f_rle);

    return _SUCCESS;
}



_modules_error get_frequencies(char * const path, const int block_size)
{
    return true;
}