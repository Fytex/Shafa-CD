/************************************************
 *
 *  Author(s): Francisco Neves, Leonardo Freitas
 *  Created Date: 3 Dec 2020
 *  Updated Date: 2 Jan 2021
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
\brief Reads the frequencies of each block provided by the .freq file 
 @param codes_input Buffer of the respective .freq file block
 @param frequencies Array to store the frequencies from each symbol
 @returns Error status
*/
static _modules_error read_block(char * restrict codes_input, unsigned long * restrict frequencies) 
{
    int read_count;
    // Checks if there are any errors with the input file
    if (sscanf(codes_input, "%lu%n;", frequencies, &read_count) != 1)
        return _FILE_UNRECOGNIZABLE;

    codes_input += read_count + 1;

    // Loop to go through all symbols
    for (int i = 1; i < NUM_SYMBOLS; ++i) {
        
        // Reads the frequency  
        if (sscanf(codes_input, "%lu%n", &frequencies[i], &read_count) == 1) {
            
            // Checks for possible errors in .freq file
            if (codes_input[read_count] != ';' && i != NUM_SYMBOLS - 1)
                return _FILE_UNRECOGNIZABLE;

            codes_input += read_count + 1;
        }  

        // Checks if we are in one of the specific cases with equal frequencies
        else if (*codes_input == ';' || (*codes_input == '\0' && i == NUM_SYMBOLS - 1)) {
            frequencies[i] = frequencies[i-1];
            ++codes_input;
        }

        // Checks for possible errors in .freq file
        else
            return _FILE_UNRECOGNIZABLE;
    }

    // Checks for possible errors in the buffer
    if (codes_input[-1] != '\0')
        return _FILE_UNRECOGNIZABLE;
    
    return _SUCCESS;
}

/**
\brief Sort the frequencies array in descending order 
 @param frequencies The array to save the frequencies
 @param positions Array with the original index of each symbol
 @param left Inital index of the array 
 @param right Final index of the array
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
        frequency value associated with each symbol isn't lost
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
\brief Calculates the sum of the positional frequencies from the first to the last position
 @param frequencies Array Frequencies 
 @param first First Element of the array
 @param last Last element of the array
 @returns Total sum of the frequencies 
*/
static unsigned long sum_freq (unsigned long frequencies[], int first, int last)
{
    unsigned long sum = 0;

    for (int i = first; i <= last; i++)
        sum += frequencies[i];

    return sum;
}

/**
\brief Find the best division of any sequence of frequencies ordered between the element in the first position and the element in the last position
 @param frequencies Array Frequencies 
 @param first First element of the array
 @param last  Last element of the array
 @returns Index of the best division
*/
static int best_Division (unsigned long frequencies[], int first, int last)
{
    
    int division = first, total , mindif, dif;
    unsigned long g1 = 0;

    total = mindif = dif = sum_freq(frequencies, first, last);

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
\brief Add character representing a bit 0 or 1 to the code according to the Shannon-Fano algorithm 
 @param value bit to be added to code (0 or 1)
 @param codes Place to store the codes
 @param start First Element
 @param end Last Element 
*/
static void add_bit_to_code(char value, char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{
    char * symbol_codes;
    /*For symbol_codes, start the pointer on the first element, 
    going through the array until the end */
    for ( ; start <= end; ++start) {
        symbol_codes = codes[start];

        // While the pointer is moved it adds value 1 or 0.
        for ( ; *symbol_codes; ++symbol_codes);
        *symbol_codes = value;
    }
}

/**
\brief Apply the Shannon-Fano algorithm 
 @param frequencies Array with frequencies
 @param codes Array to store the codes 
 @param start First element to aply the algorithm
 @param end Last element to apply the algorithm
*/
static void sf_codes (unsigned long frequencies[], char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{
    //While the pointer at the beginning is not the same as at the end it applies the algorithm
    if (start != end){
               
        int div = best_Division(frequencies, start, end);
     
        add_bit_to_code('0', codes, start, div);
        add_bit_to_code('1', codes, div + 1, end);

        sf_codes(frequencies, codes, start, div);
        sf_codes(frequencies, codes, div + 1, end);
    }
}

/**
\brief Counts how many symbols have frequencies different from 0 
 @param frequencies Array of the frequencies sorted in descending order
 @returns Number of non-null elements in the array
*/
static int not_null (unsigned long frequencies[NUM_SYMBOLS])
{
    int r = 0;

     // Parses the array from the end to the beginning until it finds the first non-null element incrementing r by 1 each interation
    for (int i = NUM_SYMBOLS - 1; frequencies[i] == 0; --i) ++r;

    return (NUM_SYMBOLS - 1 - r);
}

/**
\brief Prints in the screen all information related to this module 
 @param num_blocks Number of blocks analyzed
 @param sizes Array with sizes of each block analyzed
 @param total_time Time it took to execute the module 
 @param path Path of the created .cod file
*/
static inline void print_summary(unsigned long long num_blocks, const unsigned long * sizes, const double total_time, const char * const path)
{
    unsigned long long i;

    printf(
            "Francisco Neves,a93202,MIEI/CD, 1-JAN-2021\n"
            "Leonardo Freitas,a93281,MIEI/CD, 1-JAN-2021\n"
            "Module:T (Calculation of symbol codes)\n"
            "Number of blocks: %lu\n"
            "Size of blocks analyzed in the symbol file: " ,
            num_blocks 
    );
    // Prints the sizes of each block, except the last 
    for (i = 0; i < num_blocks - 1; ++i) {
        printf("%lu/", sizes[i]);
    }
    // Prints the size of the last block
    printf("%lu bytes\n", sizes[i]);

    printf(
            "Module runtime (milliseconds): %f\n"
            "Generated file %s\n" ,
            total_time, path
    );       
}


_modules_error get_shafa_codes(const char * path)
{
    clock_t t;
    FILE * fd_freq, * fd_codes;
    char * path_freq;
    char * path_codes;
    char * block_input;
    char mode;
    unsigned long long num_blocks = 0;
    unsigned long block_size = 0;
    int freq_notnull, iter;
    int error = _SUCCESS;
    int positions[NUM_SYMBOLS];
    unsigned long frequencies[NUM_SYMBOLS], * sizes = NULL ;
    double total_time;
    char (* codes)[NUM_SYMBOLS];

    t = clock();
    
    // add .freq extension to read the correct file
    path_freq = add_ext(path, FREQ_EXT);
 
    // Checks if it was possible to add the extension
    if (path_freq) {

        // Opens the file to read
        fd_freq = fopen(path_freq, "rb");

        // Checks if it was possible to open the file
        if (fd_freq) {

             // Reading the header of .freq file
            if (fscanf(fd_freq, "@%c@%lu", &mode, &num_blocks) == 2) {   

                // Checks if it haves a possible mode (R - RLE or N - Normal)
                if (mode == 'R' || mode == 'N') {

                // Allocates memory to an array with the purpose of saving the sizes of each block
                    sizes = malloc (num_blocks * sizeof(unsigned long));
                    
                    // Checks if it was possible to allocate memory
                    if (sizes) {                    
                        
                        // Add .cod extension to write the proper file
                        path_codes = add_ext(path, CODES_EXT);
                        
                        // Checks if it was possible to add the extension
                        if (path_codes) {

                            // Opens the file to write
                            fd_codes = fopen(path_codes, "wb");

                            // Checks if it was possible to open the file
                            if (fd_codes) {
                                
                                // Prints header in the .cod file and checks if it only prints the proper elements
                                if (fprintf(fd_codes, "@%c@%lu", mode, num_blocks) >= 3) {                               
                                    
                                    // Loop to analyze every block in .freq file
                                    for (long long i = 0; i < num_blocks && !error; ++i) {

                                        // Memory allocation to save the generated codes
                                        codes = calloc(1, sizeof(char[NUM_SYMBOLS][NUM_SYMBOLS]));
                                        
                                        // Checks if it was possible to allocate the required memory
                                        if (codes) {
                                            
                                            // Initializes the array to keep the frequencies with 0's
                                            memset(frequencies, 0, NUM_SYMBOLS * 4);

                                            // Initializes the array to keep the original index of each symbol
                                            for (int j = 0; j < NUM_SYMBOLS; ++j) positions[j] = j;

                                            // Reads the current block size and verifies possible file stream errors
                                            if (fscanf(fd_freq, "@%lu", &block_size) == 1) {

                                                // Saves the size of the block in the array to that purpose
                                                sizes[i] = block_size;
                                    
                                                // Allocates memory to keep the frequencies read, so it's possible to the lecture in only 1 access
                                                block_input = malloc(9 * NUM_SYMBOLS + (NUM_SYMBOLS - 1) + 1); // 9 (max digits for frequency) + 256 (symbols) + 255 (';') + 1 (NULL terminator)

                                                // Checks if it was possible to allocate the required memory
                                                if (block_input) {
                                                    
                                                    // Reads the frequencies and verifies the read
                                                    if (fscanf(fd_freq, "@%2559[^@]", block_input) == 1) {
                                            
                                                        // Calls read_block function
                                                        error = read_block(block_input, frequencies);
                                                       
                                                       // Checks for possible errors in read_block function
                                                        if (!error) {
                                                            
                                                            // Calls insert_sort function
                                                            insert_sort(frequencies, positions, 0, NUM_SYMBOLS - 1);

                                                            // Saves in freq_notnull the number of non-null elements in the array
                                                            freq_notnull = not_null(frequencies);

                                                            // Calls sf_codes to generate the Shannon-Fano codes
                                                            sf_codes(frequencies, codes, 0, freq_notnull);

                                                            // Prints in the .cod file the block size
                                                            if (fprintf(fd_codes, "@%lu@", block_size) >= 2) {

                                                                // Loop to print the codes till the last one in .cod file and checks for possible file stream errors
                                                                for (iter = 0; iter < NUM_SYMBOLS - 1 && !error; ++iter) {
                                                                    
                                                                    if (fprintf(fd_codes, "%s;", codes[positions[iter]]) < 1)
                                                                        error = _FILE_STREAM_FAILED;
                                                                }

                                                                // Prints last code in the file and checks for possible file stream errors
                                                                if (!error && fprintf(fd_codes, "%s", codes[positions[iter]]) < 0) 
                                                                    error = _FILE_STREAM_FAILED;
                                                                    
                                                            }
                                                            else 
                                                                error = _FILE_STREAM_FAILED;
                                                             
                                                        }

                                                    }
                                                    else 
                                                        error = _FILE_STREAM_FAILED;
                                                    
                                                    // Free allocated memory to block_input
                                                    free(block_input);
                                                }
                                                else
                                                    error = _LACK_OF_MEMORY;
                                            }
                                            else 
                                                error = _FILE_STREAM_FAILED;
                                            
                                            // Free allocated memory to codes
                                            free(codes);
                                        }
                                        else
                                            error = _LACK_OF_MEMORY;
                                    }
                                }
                                else 
                                    error = _FILE_STREAM_FAILED;

                                    /* if we don't have any error at this point, 
                                    it should write "@0" in the .cod file to indicate 
                                    that there are no more blocks*/
                                if (!error)
                                    fprintf(fd_codes, "@0");
                                
                                // Closes output file
                                fclose(fd_codes);
                            }
                            else {
                                error = _FILE_INACCESSIBLE;

                                // Free allocated memory to path_codes
                                free(path_codes);
                            }
                        }
                        else 
                            error = _LACK_OF_MEMORY;
                    }
                    else
                        error = _LACK_OF_MEMORY;      
                }
                else
                    error = _FILE_UNRECOGNIZABLE;   
            }  
            else 
                error = _FILE_UNRECOGNIZABLE;
            
            // Closes input file
            fclose(fd_freq);
        }
        else 
            error = _FILE_INACCESSIBLE;
        
        // Free allocated memory to path_freq
        free(path_freq);
    }
    else
        error = _LACK_OF_MEMORY;     

      // If no error occurred during the execution of the module, the time taken to execute it is counted 
    if (!error) {
        t = clock() - t;
        total_time = (((double) t) / CLOCKS_PER_SEC) * 1000;

        // Calls print_summary function
        print_summary(num_blocks, sizes, total_time, path_codes);
    }              

    // Free allocated memory to sizes
    free(sizes);
    
    return error;
}
