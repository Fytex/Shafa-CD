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
#include "utils/file.h"
#include "utils/errors.h"
#include "utils/extensions.h"

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
    int n_blocks; 
    if (fscanf(f_freq, "@R@%d@", &n_blocks) != 1) return _FILE_UNRECOGNIZABLE;
    
    // Reads from RLE and FREQ , while writting the decompressed version of its contents in the TXT file
    for (int i = 0; i < n_blocks; ++i) {
        int block_size;
        if (fscanf(f_freq, "%d@", &block_size) == 1) { // Reads the size of the block
            _modules_error error;
            char* buffer = load_rle(f_rle, block_size, &error); // Loads block to buffer
            if (!buffer) return error; // When buffer is NULL there was an error in load_rle that should be reported
            int size_sequence;
            char* sequence = decompress_string(buffer, block_size, &size_sequence, &error);
            free(buffer);
            int res = fwrite(sequence, 1, size_sequence, f_txt); // Writes decompressed string in txt file
            free(sequence);
            if (res != size_sequence) return _FILE_STREAM_FAILED; 
            
        }
        // Advances all the frequencies of the symbols (they are unnecessary for this process)
        if (i < n_blocks - 1) {
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

void free_tree(BTree a) {

    if (a) {
        free_tree(a->right);
        free_tree(a->left);
        free(a);
    }

}

_modules_error add_tree(BTree* decoder, char *code, char symbol) 
{
    int i;
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

_modules_error create_tree (FILE* f_cod, int * block_sizes, long long index, BTree* decoder)
{
    _modules_error error = _SUCCESS;
    // Initialize root without meaning 
    *decoder = malloc(sizeof(struct btree));
   
    if (*decoder) {
        
        (*decoder)->left = (*decoder)->right = NULL;
       
        int crrb_size; 
        // Reads the current block size
        if (fscanf(f_cod, "@%d", &crrb_size) == 1) {
           
            // Saves it to the array
            block_sizes[index] = crrb_size; 
            // Allocates memory to a block of code from .cod file
            char* code = malloc(33152);            
            if (code) {
                // Places it in the allocated string
                if (fscanf(f_cod,"@%33151[^@]s", code) == 1) {   
                         
                    for (int k = 0, l = 0; code[l];) {
                        // When it finds a ';' it is no longer on the same symbol. This updates it.
                        while (code[l] == ';') {
                            k++;
                            l++;
                        }
                        // Allocates memory for the sf code of a symbol
                        char* sf = malloc(257);
                        if (sf) {           
                            int j;
                            for (j = 0; code[l] && (code[l] != ';'); ++j, ++l) 
                                sf[j] = code[l];
                            sf[j] = '\0';           
                            if (j != 0) {
                                // Adds the code to the tree
                                error = add_tree(decoder, sf, k);
                                if (error != _SUCCESS) break;
                              
                            }   
                            free(sf);        
                        }
                        else {    

                            error = _LACK_OF_MEMORY;

                        }
                      
                    }
                }
                else {
                    
                    error = _FILE_STREAM_FAILED; 
                     
                }
                free(code);
            }
            else {    

                error = _LACK_OF_MEMORY;
              
            }          
              
        }
        else  {    

            error = _FILE_STREAM_FAILED;

        }
        
        free(*decoder);
    }
    else {

        error = _LACK_OF_MEMORY;

    }

    
    return error;
}

_modules_error shafa_decompress (char ** const path) {

    _modules_error error = _SUCCESS;

    FILE* f_shafa = fopen(*path, "rb");
    if (f_shafa) {

        char* path_cod = rm_ext(*path);
        if (path_cod) {

            path_cod = add_ext(path_cod, ".cod");
            if (path_cod) {

                FILE* f_cod = fopen(path_cod, "rb");
                if (f_cod) {

                    char* path_wrt = rm_ext(*path);
                    if (path_wrt) {

                        FILE* f_wrt = fopen(path_wrt, "wb");
                        if (f_wrt) {

                            long long n_blocks;
                            if (fscanf(f_shafa, "@%lld", &n_blocks) == 1) {

                                char mode;
                                if (fscanf(f_cod, "@%c@%lld", &mode, &n_blocks)) {
                                    int* block_sizes = malloc(sizeof(int)*n_blocks);

                                    if ((mode == 'N') || (mode == 'R')) {
                                    
                                        int l = 0;
                                        for (long long i = 0; i < n_blocks; ++i) {

                                            BTree decoder;
                                            error = create_tree(f_cod, block_sizes, i, &decoder);
                
                                            if (error == _SUCCESS) {
                                                
                                                BTree root = decoder;
                                                int sf_bsize;
                                                if (fscanf(f_shafa, "@%d@", &sf_bsize) == 1) {

                                                    char* shafa_code = malloc(sf_bsize+1);
                                                    if (shafa_code) {

                                                        char* decomp = malloc(block_sizes[i]+1);
                                                        if (decomp) {

                                                            if (fread(shafa_code, 1, sf_bsize, f_shafa) == sf_bsize) {

                                                                for (int j = 0; shafa_code[j]; ++j) {
                                                                    
                                                                    if (shafa_code[j] == '0') decoder = decoder->left;
                                                                    else decoder = decoder->right;
                                                                    if (decoder && !(decoder->left) && !(decoder->right)) { // PROBLEM: KEEPS GOING TO 6
                                                        
                                                                        decomp[l++] = decoder->symbol;
                                                                        decoder = root;

                                                                    }

                                                                }
                                                                decomp[++l] = '\0';
                                                                --l;
                                                                if (fwrite(decomp, 1, l, f_wrt) != l) {

                                                                    error = _FILE_STREAM_FAILED;
                                                                    i = n_blocks;

                                                                }

                                                            }
                                                            else {

                                                                error = _FILE_STREAM_FAILED;
                                                                i = n_blocks;

                                                            }

                                                            free(decomp);
                                                        }
                                                        else {

                                                            error = _LACK_OF_MEMORY;
                                                            i = n_blocks;

                                                        }

                                                        free(shafa_code);
                                                    }
                                                    else {

                                                        error = _LACK_OF_MEMORY;
                                                        i = n_blocks;

                                                    }

                                                }
                                                else {

                                                    error = _FILE_STREAM_FAILED;

                                                }

                                            free_tree(decoder);

                                            }
                                            else {

                                                i = n_blocks;

                                            }

                                        }

                                    }
                                    else {

                                        error = _FILE_UNRECOGNIZABLE;

                                    }
                                    
                                }
                                else {

                                    error = _FILE_STREAM_FAILED;

                                }

                            }
                            else {
                                
                                error = _FILE_STREAM_FAILED;

                            }
                            fclose(f_wrt);

                        }
                        else {

                            error = _FILE_UNRECOGNIZABLE;

                        }
                        free(path_wrt);

                    }
                    else {

                        error = _LACK_OF_MEMORY;

                    }
                    fclose(f_cod);

                }
                else {
                
                    error = _FILE_UNRECOGNIZABLE;

                }

            }
            else {

                error = _LACK_OF_MEMORY;

            }
            free(path_cod);

        }
        else {

            error = _LACK_OF_MEMORY;

        }

        fclose(f_shafa);

    }
    else {
        
        error = _FILE_UNRECOGNIZABLE;

    }

    return error;
}

