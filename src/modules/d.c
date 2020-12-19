/***************************************************
 *
 *  Author(s): Alexandre Martins, Beatriz Rodrigues
 *  Created Date: 3 Dec 2020
 *  Updated Date: 13 Dec 2020
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

#define n_simb 256

/*
Write details about functions in here
*/

char* load_rle (FILE* f_rle, int size_of_block, _modules_error* error) 
{
    char* buffer = malloc(sizeof(char)*size_of_block+1); 
    if (!buffer) {
        *error = _LACK_OF_MEMORY;
        return NULL;
    }
    int res = fread(buffer, 1, size_of_block, f_rle);
    if (res != size_of_block) {
        *error = _FILE_STREAM_FAILED;
        return NULL;
    }
    buffer[res] = '\0';
    return buffer;
}

char* decompress_string (char* buffer, int block_size, int* size_string, _modules_error* error) {
    int orig_size;
    // Assumption of the smallest size possible for the decompressed file
    if (block_size <= _64KiB) orig_size = _64KiB + _1KiB;
    else if (block_size <= _640KiB) orig_size = _640KiB + _1KiB;
    else if (block_size <= _8MiB) orig_size = _8MiB + _1KiB;
    else orig_size = _64MiB + _1KiB;

    // Allocation of the corresponding memory
    char* sequence = malloc(sizeof(char)*orig_size);
    if (!sequence) {
        *error = _LACK_OF_MEMORY;
        return NULL;
    }

    // Process of decompression
    int l = 0;
    for (int j = 0; j < block_size; ++j) {
        char simb = buffer[j];
        if (!simb) { // Detects format "00 symbol number_of_repetitions"
            simb = buffer[++j]; // Finds the symbol that repeats 
            int n_reps = buffer[++j]; // Saves the number of repetitions

            // VERIFICAR ISTO
            // If the memory wasn't enough allocate more
            if (l + n_reps >= orig_size) {
                switch (orig_size) {
                    case _64KiB + _1KiB:
                        block_size = _640KiB + _1KiB;
                        break;
                    case _640KiB + _1KiB:
                        block_size = _8MiB + _1KiB;
                        break;
                    case _8MiB + _1KiB:
                        block_size = _64MiB + _1KiB;
                        break;
                    default:
                        *error = _FILE_UNRECOGNIZABLE;
                        return NULL;
                }
                sequence = realloc(sequence, block_size);
                if (!sequence) {
                    free(sequence);
                    *error = _LACK_OF_MEMORY;
                    return NULL;
                }
            }
            memset(sequence + l, simb, n_reps); // Places the symbol along the string according to the number of repetitions
            l += n_reps; // Advances the index to one that hasn't been filled
        }
        else { // Case where there aren't repetitions
            sequence[l] = simb; 
            ++l;
            // Pensar se tenho que por o caso de alocar mais memoria aqui
        }
    }
    *size_string = l;
    sequence[++l] = '\0';

    return sequence;
}

_modules_error rle_decompress(char** const path) 
{
    // Opening the files
    FILE* f_rle = fopen(*path, "rb");
    if (!f_rle) return _FILE_INACCESSIBLE;
    FILE* f_freq = fopen(add_ext(*path, FREQ_EXT), "rb");
    if (!f_freq) {
        fclose(f_rle);
        return _FILE_INACCESSIBLE;
    }
    FILE* f_txt = fopen(rm_ext(*path), "wb");
    if (!f_txt) {
        fclose(f_rle);
        fclose(f_freq);
        return _FILE_INACCESSIBLE;
    }
    
    // Reads header
    int n_blocos; 
    if (fscanf(f_freq, "@R@%d@", &n_blocos) != 1) return _FILE_UNRECOGNIZABLE;
    
    // Reads from RLE and FREQ , while writting the decompressed version of its contents in the TXT file
    for (int i = 0; i < n_blocos; ++i) {
        int block_size;
        if (fscanf(f_freq, "%d@", &block_size) == 1) { // Reads the size of the block
            printf("Block size: %d\n", block_size);
            _modules_error error;
            char* buffer = load_rle(f_rle, block_size, &error); // Loads block to buffer
            if (!buffer) return error; // When buffer is NULL there was an error in load_rle that should be reported
            int size_sequence;
            char* sequence = decompress_string(buffer, block_size, &size_sequence, &error);
            free(buffer);
            int res = fwrite(sequence, 1, size_sequence, f_txt); // Writes decompressed string in txt file
            if (res != size_sequence) return _FILE_STREAM_FAILED; 
            free(sequence);
        }
        // Advances all the frequencies of the symbols (they are unnecessary for this process)
        if (i < n_blocos - 1) {
            for (int k = 0, c; k < n_simb; ++k) { 
                if (fscanf(f_freq, "%d;", &block_size) == 0) {
                    c = fgetc(f_freq);
                    if (c != ';' && c != '@') return _FILE_UNRECOGNIZABLE;
                }
            }
        }
    }

    // Closes all the files
    fclose(f_rle);
    fclose(f_freq);
    fclose(f_txt);

    return _SUCCESS;
}

/**
\brief struct of a btree to save the symbols codes
*/
typedef struct btree{
    char symbol;
    struct btree *left,*right;
} *BTree;

_modules_error add_tree(BTree* decoder, char *code, char symbol) 
{
    int i = 0;
    for (i = 0; code[i]; ++i) { 
        if (*decoder && code[i] == '0') decoder = &(*decoder)->left;
        else if (*decoder && code[i] == '1') decoder = &(*decoder)->right;
        else {
            *decoder = malloc(sizeof(struct btree));
            if (!(*decoder)) return _LACK_OF_MEMORY;
            (*decoder)->left = (*decoder)->right = NULL;
            if (code[i] == '0') decoder = &(*decoder)->left;
            else decoder = &(*decoder)->right;
        } 
    }
    *decoder = malloc(sizeof(struct btree));
    (*decoder)->symbol = symbol;
    (*decoder)->left = (*decoder)->right = NULL;
    return _SUCCESS;
}

_modules_error create_tree(char *path, int **blocks_sizes, int *size, BTree *decoder) 
{
    // Creates a root without meaning
    *decoder = malloc(sizeof(struct btree));
    if (!(*decoder)) return _LACK_OF_MEMORY;
    (*decoder)->left = (*decoder)->right = NULL;

    // Opens cod file
    char* name_cod = add_ext(rm_ext(path), ".cod");
    FILE* f_cod = fopen(name_cod, "rb");
    if (!f_cod) return _FILE_INACCESSIBLE;

    // Reads header
    if (fscanf(f_cod, "@R@%d", size) != 1) return _FILE_UNRECOGNIZABLE;
    *blocks_sizes = malloc (sizeof(int)*(*size));
    if (!(*blocks_sizes)) return _LACK_OF_MEMORY;

    // Works block by block
    for (int i = 0; i < *size; ++i) {
        // Saves block size in an array for future purposes in rle decompress
        int block_size;
        if (fscanf(f_cod, "@%d", &block_size) == 1) 
            *blocks_sizes[i] = block_size;
        else return _FILE_UNRECOGNIZABLE;
        
        // Allocates memory for one block of sf codes from .cod
        char* code = malloc(33151);
        if (!code) return _LACK_OF_MEMORY;
        // Loads one block 
        fscanf(f_cod,"@%[^@]s", code);

        for (int k = 0, l = 0; code[l];) {
            while (code[l] == ';') {k++;l++;} // Ignores ; but sinalizes that we changed symbol
            char* sf = malloc(33); // Allocates memory for the code of one symbol
            if (!sf) return _LACK_OF_MEMORY;
            int j;
            for (j = 0; code[l] && code[l] != ';'; ++j)  // Copies the code of one symbol to another string
                sf[j] = code[l++];
            sf[j] = '\0';
            _modules_error error;
            if (j!=0) error = add_tree(decoder, sf, k); // Adds the symbol to the tree
            if (!(*decoder)) return error; 
        }
    }

    return _SUCCESS;
}

_modules_error shafa_decompress(char ** const path)
{
    return _SUCCESS;
}

