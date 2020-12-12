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

char* loadingInfo (FILE* f_rle, int block_size) 
{
    char* buffer = malloc(sizeof(char)*block_size+1); 
    if (!buffer) return NULL;
    int res = fread(buffer, 1, block_size, f_rle); // Loads a block of RLE file to string
    if (res != block_size) return NULL;
    buffer[res] = '\0';
    return buffer;
}

char* decompress_string (char* buffer, int block_size, int* size_sequence) 
{
    char* sequence = malloc(5000); // CORRIGIR: arranjar algo mais eficiente
    int l = 0;
    for (int j = 0; j < block_size; ++j) {
        char simb = buffer[j];
        if (!simb) { // Detects format "{0} symbol {number_of_repetitions}"
            simb = buffer[++j]; // Finds the symbol that repeats 
            int n_reps = buffer[++j]; // Saves the number of repetitions
            memset(sequence + l, simb, n_reps); // Places the symbol along the string according to the number of repetitions
            l += n_reps; // Advances the index to one that hasn't been filled
        }
        else { // Case where there aren't repetitions
            sequence[l] = simb; 
            ++l;
        }
    }
    *size_sequence = l; // Saves the size of the decompressed string
    sequence[++l] = '\0'; 
    return sequence;
}

_modules_error rle_decompress2(char** const path) 
{
    // Opening the files
    FILE* f_rle = fopen(*path, "rb");
    if (!f_rle) return _FILE_INACCESSIBLE;
    FILE* f_freq = fopen(add_ext(*path, ".freq"), "rb");
    if (!f_freq) return _FILE_INACCESSIBLE;
    FILE* f_txt = fopen(rm_ext(*path), "wb");
    if (!f_txt) return _FILE_INACCESSIBLE;
    
    // Reads header
    int n_blocos; 
    char mode[2];
    if (fscanf(f_freq, "@%c@%d@", mode, &n_blocos) != 2) return _FILE_CORRUPTED;
    if (mode[0] != 'R') return _OUTSIDE_MODULE;
    
    // Reads from RLE and FREQ , while writting the decompressed version of its contents in the TXT file
    for (int i = 0; i < n_blocos; ++i) {
        int block_size;
        if (fscanf(f_freq, "%d[^@]", &block_size) == 1) { // Reads the size of the block
            char* buffer = loadingInfo(f_rle, block_size); // Loads block to buffer
            if (!buffer) return _LACK_OF_MEMORY;
            int size_sequence;
            char* sequence = decompress_string(buffer, block_size, &size_sequence);
            free(buffer);
            int res = fwrite(sequence, 1, size_sequence, f_txt); // Writes decompressed string in txt file
            if (res != size_sequence) return _FILE_CORRUPTED; 
            free(sequence);
        }
        // Advances all the frequencies of the symbols (they are unnecessary for this process)
        int block;
        if (i < n_blocos - 1)
            for (int k = 0; k <= 256; ++k) {
                fscanf(f_freq, "%d[^;]", &block);// corrigir: ignores return value
                fseek(f_freq, 1, SEEK_CUR);
            }
        
    }

    // Closes all the files
    fclose(f_rle);
    fclose(f_freq);
    fclose(f_txt);

    return _SUCCESS;
}

_modules_error shafa_decompress(char ** const path)
{
    return _SUCCESS;
}