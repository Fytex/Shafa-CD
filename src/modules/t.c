/************************************************
 *
 *  Author(s): Francisco Neves, Leonardo Freitas
 *  Created Date: 3 Dec 2020
 *  Updated Date: 19 Dec 2020
 *
 ***********************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "t.h"
#include "utils/extensions.h"

#define NUM_SYMBOLS 256
#define MIN(a,b) ((a) < (b) ? a : b)

void read_Block (char * codes_input, uint32_t frequencies[NUM_SYMBOLS]) 
{
    char * ptr = codes_input;
    uint32_t auxfreq;
    int nread=0;

    for (int i = 0; i < NUM_SYMBOLS; ++i){
        if ( sscanf(ptr, "%u[^;]",&frequencies[i]) == 1 ){
                     
            auxfreq = frequencies[i];
             
            while (auxfreq != 0){  
                auxfreq /= 10;  
                nread++;  
            }  
            if (nread == 0) ++nread;

            ptr += nread + 1; 
            nread = 0;
        }       
        else if(i >= 1){
            frequencies[i] = frequencies[i-1];
            ++ptr;
        }
        else{
            ++ptr;
        }
    }
}

void insert_Sort (uint32_t frequencies[], int positions[], int left, int right)
{
    uint32_t tmpFreq;
    int j, a, iters;

    for (int i = left + 1; i <= right; ++i){

        tmpFreq = frequencies[i];
        j = i - 1;

        while (j >= left && frequencies[j] < tmpFreq){
            frequencies[j+1] = frequencies[j];           
            --j;
        }
        frequencies[j+1] = tmpFreq;
        a = j + 1;        
        iters = i - a;

        for (int idx = i-1; iters; --idx)
            if (positions[idx] >= a) {
                ++positions[idx];
                --iters;
            }
        positions[i] = a;       
    }
} 


uint32_t sum_Freq (uint32_t frequencies[], int first, int last)
{
    uint32_t soma = 0;

    for (int i = first; i <= last ; i++)
        soma += frequencies[i];

    return soma;
}


int best_Division (uint32_t frequencies[], int first, int last)
{
    
    int division = first, total , mindif, dif;
    uint32_t g1 = 0;

    total = mindif = dif = sum_Freq(frequencies,first,last);

    while (dif == mindif){
        g1 = g1 + frequencies[division];
        dif = abs(2*g1 -total);
            if (dif < mindif){
                division = division + 1 ;
                mindif = dif;
            }
            else
                dif = mindif +1 ;
    }

    return division - 1;
}

void add_bit_to_code(char value, char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{
    char * symbol_codes;

    for ( ; start <= end; ++start) {
        symbol_codes = codes[start];
        for ( ; *symbol_codes; ++symbol_codes);
        *symbol_codes = value;
    }
}


void sf_codes (uint32_t frequencies[], char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{
    if (start != end){
               
        int div = best_Division(frequencies, start, end);
     
        add_bit_to_code('0', codes, start, div);
        add_bit_to_code('1', codes, div + 1, end);

        sf_codes(frequencies, codes, start, div);
        sf_codes(frequencies, codes, div + 1, end);
    }
}

int not_Null (uint32_t frequencies[NUM_SYMBOLS])
{
    int r = 0;
    for (int i = NUM_SYMBOLS - 1; frequencies[i] == 0; --i) ++r;

    return (NUM_SYMBOLS - 1 - r);
}


// This module should receive a .freq file, but shafa.c will handle that file and pass to this module the original file

_modules_error get_shafa_codes(const char * path)
{
    clock_t t;
    t = clock();
    FILE * fd_file, * fd_freq, * fd_codes;
    char * path_freq;
    char * path_codes;
    char * block_input;
    char mode;
    long long num_blocks = 0;
    unsigned long block_size = 0;
    int freq_notnull;
    int error = _SUCCESS;
    int positions[NUM_SYMBOLS];
    uint32_t frequencies[NUM_SYMBOLS] ;
    


    path_freq = add_ext (path, FREQ_EXT);

    if (path_freq){

        fd_freq = fopen ("path_freq", "rb");

        if (fd_freq){

            if (fscanf(fd_freq, "@%c@%lld", &mode, &num_blocks) == 2){

                if (mode != 'R' && mode != 'N')
                    return _FILE_UNRECOGNIZABLE;
                

                fd_file = fopen(path, "rb");

                if (fd_file){

                    path_codes = add_ext (path, CODES_EXT);

                    if (path_codes){

                        fd_codes = fopen (path_codes, "wb");

                        if (fd_codes){

                            fprintf (fd_codes, "@%c@%lld", mode, num_blocks);

                            for (long long i = 0; i < num_blocks; ++i) {

                                char (* codes)[NUM_SYMBOLS] = calloc (1, sizeof(char [NUM_SYMBOLS][NUM_SYMBOLS]));

                                memset(frequencies, 0, NUM_SYMBOLS*4);
                                for (int j = 0; j < NUM_SYMBOLS; ++j)positions[j] = j;

                                fscanf(fd_freq, "@%lu@", &block_size);
                                block_input = malloc (9 * 256 + 255);

                                fscanf (fd_freq, "@%[^@]", block_input);
                                
                                read_Block(block_input, frequencies);
                                insert_Sort(frequencies, positions, 0, 255);

                                freq_notnull = not_Null(frequencies);

                                sf_codes(frequencies, codes, 0, freq_notnull);

                                fprintf(fd_codes, "%d@", block_size);
                                for (int i = 0; i < NUM_SYMBOLS; ++i) fprintf(fd_codes, "%s;", codes[positions[i]]);

                                free(block_input);
                                free(codes);

                            }
                            fprintf(fd_codes, "@0");
                            
                            t = clock() - t;
                             double time_taken = ((double) t)/CLOCKS_PER_SEC;
                             double total_time = time_taken*1000;

                            puts(
                                 "Francisco Neves,a93202,MIEI/CD, 1-JAN-2021\n"
                                 "Leonardo Freitas,a93281,MIEI/CD, 1-JAN-2021\n"
                                 "Módule:T (Calculation of symbol codes)\n"
                                );
                            printf("Number of blocks: %lld\n", num_blocks);    
                            printf("Size of blocks analyzed in the symbol file: %lu\n", block_size);
                            printf("Module runtime (milliseconds): %f\n", total_time); 
                            printf("Generated file %s\n", path_codes);

                            fclose(fd_codes);
                                                       

                        }
                        else {
                            free (path_codes);
                            error = _FILE_INACCESSIBLE;
                        }
                   
                    }
                    else
                        error = _LACK_OF_MEMORY;

                    fclose(fd_file);
                    
                }
                else
                    error = _FILE_INACCESSIBLE;
            }
            else 
                error = _FILE_UNRECOGNIZABLE;
            
            fclose(fd_freq);
        }
        else 
            error = _FILE_INACCESSIBLE;
        
        free(path_freq);

    }
    else
        error = _LACK_OF_MEMORY;    

    return error;
