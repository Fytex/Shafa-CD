/***************************************************
 *
 *  Author(s): Alexandre Martins, Beatriz Rodrigues
 *  Created Date: 3 Dec 2020
 *  Updated Date: 8 Dec 2020
 *
 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "d.h"
#include "file.h"
#include "errors.h"
#include "extensions.h"

/*
Write details about functions in here
*/

_modules_error rle_decompress(char** const path) 
{
    /* 
    This part allows reading and loading file into the buffer
    */
    // Opens file in reading (binary) mode
    char* file_name = *path; 
    FILE* f_rle = fopen(file_name, "rb");
    if (!f_rle) return _FILE_INACCESSIBLE;

    // Finds the size of the RLE file
    int seek = fseek(f_rle, 0, SEEK_END); // Goes to the end of the file
    if (!seek) return _FILE_INACCESSIBLE;
    int size_f = ftell(f_rle); // Saves the size of the file 
    rewind(f_rle); // Goes back to the beggining of the file

    // Loads the contents of the RLE file to the buffer
    char* buffer = malloc(sizeof(char)*size_f); // Allocates memory for all the characters in the file
    if (!buffer) return _LACK_OF_MEMORY; 
    int res = fread(buffer, 1, size_f, f_rle); // Loads the contents of the file into the buffer
    if (res != size_f) return _FILE_CORRUPTED; 

    fclose(f_rle); // RLE file isn't necessary anymore


    /* 
    This part allows writing in the new file that will match the original file
    */
    // Removes the .RLE extension and opens the file in writing (binary) mode
    char* new_file = *path; 
    char* new = rm_ext(new_file);
    FILE* f_origin = fopen(new, "wb");
    free(new);
    if (!f_origin) return _FILE_INACCESSIBLE;  
    
    // Creats a string with enough memory to store the decompressed contents of the file
    int size_origin = 5000 ; // **** Nota: corrigir isto para uma função que leia do .FREQ o tamanho original descomprimido (reutilizada do modulo C)
    char* sequence = malloc(sizeof(char)*size_origin + 1); // Allocates memory to the decompressed content of the file
    if (!sequence) return _LACK_OF_MEMORY;
    
    // Loads the decompressed contents of the file to said string
    int l = 0; // Index of string 
    for (int i = 0; i < size_f; ++i) { // Reads RLE file and decompresses it
        char simb = buffer[i];
        if (!simb) { // Detects format "00 symbol number_of_repetitions"
            simb = buffer[++i]; // Finds the symbol that repeats 
            int n_reps = buffer[++i]; // Saves the number of repetitions
            memset(sequence + l, simb, n_reps); // Places the symbol along the string according to the number of repetitions
            l += n_reps; // Advances the index to one that hasn't been filled
        }
        else { // Case where there aren't repetitions
            sequence[l] = simb; 
            ++l;
        }
    }
    sequence[++l] = '\0'; 

    // Writes the string on the files
    fwrite(sequence, 1, --l, f_origin); // Writes the string in the file

    free(sequence);
    fclose(f_origin);
    return _SUCCESS;
}

_modules_error shafa_decompress(char ** const path)
{
    return _SUCCESS;
}