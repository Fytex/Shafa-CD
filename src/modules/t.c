/************************************************
 *
 *  Author(s): Francisco Neves, Leonardo Freitas
 *  Created Date: 3 Dec 2020
 *  Updated Date: 1 Jan 2021
 *
 ***********************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "utils/errors.h"
#include "utils/extensions.h"

#define NUM_SYMBOLS 256
#define MIN(a,b) ((a) < (b) ? a : b)

/**
 * @brief Function that reads the blocks provided by the freq file 
 * 
 * @param codes_input buffer of respective file.freq block
 * @param frequencies Array to store the freq from the symbol
 * @return _modules_error 
 */
static _modules_error read_Block(char * restrict codes_input, unsigned long * restrict frequencies) 
{
    int read_count;
    //check if something is read in relation to the frequencies of the .freq file
    if (sscanf(codes_input, "%lu%n;", frequencies, &read_count) != 1)
        return _FILE_UNRECOGNIZABLE;

    codes_input += read_count + 1;

    for (int i = 1; i < NUM_SYMBOLS; ++i) {

        if (sscanf(codes_input, "%lu%n", &frequencies[i], &read_count) == 1) {

            if (codes_input[read_count] != ';' && i != NUM_SYMBOLS - 1)
                return _FILE_UNRECOGNIZABLE;

            codes_input += read_count + 1;
        }  
        else if (*codes_input == ';' || (*codes_input == '\0' && i == NUM_SYMBOLS - 1)) {
            frequencies[i] = frequencies[i-1];
            ++codes_input;
        }
        else
            return _FILE_UNRECOGNIZABLE;
    }

    if (codes_input[-1] != '\0')
        return _FILE_UNRECOGNIZABLE;
    
    return _SUCCESS;
}

/**
 * @brief Fuction to sort the frequencies array in descending order
 * 
 * @param frequencies The array we used
 * @param positions Array with the Symbols Positions
 * @param start Begining of the array 
 * @param end  Ending of the array
 */
static void insert_sort (unsigned long frequencies[], int positions[], int left, int right)
{
    unsigned long tmpFreq;
    int j, a, iters;

    for (int i = left + 1; i <= right; ++i){

        tmpFreq = frequencies[i];
        j = i - 1;

        /* Move elements of frequencies[0..i-1], that are greater
        than tmpFreq, to one position ahead of their current position
        */
        while (j >= left && frequencies[j] < tmpFreq){
            frequencies[j+1] = frequencies[j];           
            --j;
        }
        frequencies[j+1] = tmpFreq;
        a = j + 1;        
        iters = i - a;
         /* Do the same thing to position so that the 
        frequency value associated with each symbol is not lost
        */
        for (int idx = i-1; iters; --idx)
            if (positions[idx] >= a) {
                ++positions[idx];
                --iters;
            }
        positions[i] = a;       
    }
} 

/**
 * @brief Function that calculates 
 * the sum of the positional frequencies first 
 * to the last position.
 * 
 * @param frequencies Array Frequencies 
 * @param first First Element of the array
 * @param last Last element of the array
 * @return unsigned long 
 */
static unsigned long sum_Freq (unsigned long frequencies[], int first, int last)
{
    unsigned long sum = 0;

    for (int i = first; i <= last; i++)
        sum += frequencies[i];

    return sum;
}

/**
 * @brief The function is used to calculate the best division of 
 * any sequence of frequencies ordered between the element in position first and the element in position last, as long as last > first.
 * 
 * @param frequencies Array Frequencies 
 * @param first First element of the array
 * @param last  Last element of the array
 * @return int 
 */
static int best_Division (unsigned long frequencies[], int first, int last)
{
    
    int division = first, total , mindif, dif;
    unsigned long g1 = 0;

    total = mindif = dif = sum_Freq(frequencies, first, last);

    while (dif == mindif){

        g1 = g1 + frequencies[division];
        dif = abs(2 * g1 - total);

            if (dif < mindif){
                division = division + 1;
                mindif = dif;
            }
            else
                dif = mindif + 1;
    }

    return division - 1;
}

/**
 * @brief Function that will add the bit 0 or 1 to the code according to the Sf algorithm
 * 
 * @param value bit will be added to code 0 or 1
 * @param codes Array to store the sf_codes
 * @param start First Element
 * @param end Last Element 
 */
static void add_bit_to_code(char value, char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{
    char * symbol_codes;
    /*For symbol_codes, start the pointer on the first element, 
    going through the array until the end */
    for ( ; start <= end; ++start) {
        symbol_codes = codes[start];
        //While the pointer is moved it adds value 1 or 0. This step is made by this for
        for ( ; *symbol_codes; ++symbol_codes);
        *symbol_codes = value;
    }
}

/**
 * @brief Function that applies the shannon fanon algorithm
 * 
 * @param frequencies Array with frequencies
 * @param codes Array to store the codes after SF
 * @param start First element of the array
 * @param end Last element of the array
 */
static void sf_codes (unsigned long frequencies[], char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{//While the pointer at the beginning is not the same as at the end it applies the algorithm
    if (start != end){
               
        int div = best_Division(frequencies, start, end);
     
        add_bit_to_code('0', codes, start, div);
        add_bit_to_code('1', codes, div + 1, end);

        sf_codes(frequencies, codes, start, div);
        sf_codes(frequencies, codes, div + 1, end);
    }
}

/**
 * @brief Function that counts how many non-null elements there are
 * 
 * @param frequencies Array for check
 * @return int 
 */
static int not_null (unsigned long frequencies[NUM_SYMBOLS])
{
    int r = 0;
     //Checks for null elements in the frequencies array and updates the r when it finds one
    for (int i = NUM_SYMBOLS - 1; frequencies[i] == 0; --i) ++r;

    return (NUM_SYMBOLS - 1 - r);
}

/**
 * @brief Gives print in the screen of all the information related to this module
 * 
 * @param num_blocks Number of Blocks in the freq file
 * @param sizes Array with all block sizes
 * @param total_time Time it took to execute the module 
 * @param path Original/RLE file's path
 */
static inline void print_summary(const long long num_blocks, const unsigned long * sizes, const double total_time, const char * const path)
{
    long long i;

    printf(
            "Francisco Neves,a93202,MIEI/CD, 1-JAN-2021\n"
            "Leonardo Freitas,a93281,MIEI/CD, 1-JAN-2021\n"
            "Module:T (Calculation of symbol codes)\n"
            "Number of blocks: %lld\n"
            "Size of blocks analyzed in the symbol file: " ,
            num_blocks 
    );
    //For -> Save the sizes of all blocks in an array and then print on the interface
    for (i = 0; i < num_blocks - 1; ++i) {
        printf("%lu/", sizes[i]);
    }
    printf("%lu bytes\n", sizes[i]);

    printf(
            "Module runtime (milliseconds): %f\n"
            "Generated file %s\n" ,
            total_time, path
    );       
}


// This module should receive a .freq file, but shafa.c will handle that file and pass to this module the original file
/**
 * @brief Get the shafa codes object/Creates a table of Shanon Fano's codes and saves it to disks
 * 
 * @param path Original/RLE file's path
 * @return _modules_error 
 */
_modules_error get_shafa_codes(const char * path)
{
    clock_t t;
    FILE * fd_freq, * fd_codes;
    char * path_freq;
    char * path_codes;
    char * block_input;
    char mode;
    long long num_blocks = 0;
    unsigned long block_size = 0;
    int freq_notnull, iter;
    int error = _SUCCESS;
    int positions[NUM_SYMBOLS];
    unsigned long frequencies[NUM_SYMBOLS], * sizes = NULL ;
    double total_time;
    char (* codes)[NUM_SYMBOLS];

    t = clock();

    path_freq = add_ext(path, FREQ_EXT);

    if (path_freq) {

        fd_freq = fopen(path_freq, "rb");

        if (fd_freq) {
             // Reading header of freq file
            if (fscanf(fd_freq, "@%c@%lld", &mode, &num_blocks) == 2) {                
                // Checking the mode of the file
                if (mode == 'R' || mode == 'N') {
                // Allocates memory to an array with the purpose of saving the sizes of each RLE/TXT block
                    sizes = malloc (num_blocks * sizeof(unsigned long));
   
                    if (sizes) {                    

                        path_codes = add_ext(path, CODES_EXT);

                        if (path_codes) {

                            fd_codes = fopen(path_codes, "wb");

                            if (fd_codes) {
                                
                                if (fprintf(fd_codes, "@%c@%lld", mode, num_blocks) >= 3) {                               
                                    
                                    for (long long i = 0; i < num_blocks && !error; ++i) {

                                        codes = calloc(1, sizeof(char[NUM_SYMBOLS][NUM_SYMBOLS]));
                                
                                        if (codes) {

                                            memset(frequencies, 0, NUM_SYMBOLS * 4);
                                            for (int j = 0; j < NUM_SYMBOLS; ++j) positions[j] = j;

                                            if (fscanf(fd_freq, "@%lu", &block_size) == 1) {
                                                sizes[i] = block_size;
                                    
                                                block_input = malloc(9 * NUM_SYMBOLS + (NUM_SYMBOLS - 1) + 1); // 9 (max digits for frequency) + 256 (symbols) + 255 (';') + 1 (NULL terminator)

                                                if (block_input) {
                                                    
                                                    if (fscanf(fd_freq, "@%2559[^@]", block_input) == 1) {
                                            
                                                        error = read_Block(block_input, frequencies);
                                                       
                                                        if (!error) {
                                                            
                                                            insert_sort(frequencies, positions, 0, NUM_SYMBOLS - 1);

                                                            freq_notnull = not_null(frequencies);

                                                            sf_codes(frequencies, codes, 0, freq_notnull);

                                                            if (fprintf(fd_codes, "@%lu@", block_size) >= 2) {

                                                                for (iter = 0; iter < NUM_SYMBOLS - 1 && !error; ++iter) {
                                                                    
                                                                    if (fprintf(fd_codes, "%s;", codes[positions[iter]]) < 1)
                                                                        error = _FILE_STREAM_FAILED;
                                                                }
                                                                    
                                                                if (!error && fprintf(fd_codes, "%s", codes[positions[iter]]) < 0) 
                                                                    error = _FILE_STREAM_FAILED;
                                                                    
                                                            }
                                                            else 
                                                                error = _FILE_STREAM_FAILED;
                                                             
                                                        }

                                                    }
                                                    else 
                                                        error = _FILE_STREAM_FAILED;
                                                    

                                                    free(block_input);
                                                }
                                                else
                                                    error = _LACK_OF_MEMORY;
                                            }
                                            else 
                                                error = _FILE_STREAM_FAILED;
                                            

                                            free(codes);
                                        }
                                        else
                                            error = _LACK_OF_MEMORY;
                                    }
                                }
                                else 
                                    error = _FILE_STREAM_FAILED;
                                /*If fd_codes have errors the program returns a warning
                                 matches the errors present in the else of this if*/
                                if (!error)
                                    fprintf(fd_codes, "@0");

                                fclose(fd_codes);
                            }
                            else {
                                error = _FILE_INACCESSIBLE;
                                free(path_codes);
                            }
                        }
                        else 
                            error = _LACK_OF_MEMORY;
                            
                        free(sizes);
                    }
                    else
                        error = _LACK_OF_MEMORY;      
                }
                else
                    error = _FILE_UNRECOGNIZABLE;   
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
      /*If no error occurred during the execution of the module, the time taken to execute it is counted 
    and the information related to it is printed (print_summary)*/
    if (!error) {
        t = clock() - t;
        total_time = (((double) t) / CLOCKS_PER_SEC) * 1000;

        print_summary(num_blocks, sizes, total_time, path_codes);
    }              

    return error;
}