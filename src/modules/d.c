/***************************************************
 *
 *  Author(s): Alexandre Martins, Beatriz Rodrigues
 *  Created Date: 3 Dec 2020
 *  Updated Date: 24 Dec 2020
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


char* load_rle (FILE* f_rle, unsigned long size_of_block, _modules_error* error) 
{
    char* buffer = malloc(size_of_block + 1);
    // Memory allocation was successful
    if (buffer) {
        // The amount of bytes read and the amount of bytes that were supposed to be read match
        if (fread(buffer,1,size_of_block, f_rle) == size_of_block) {

            buffer[size_of_block] = '\0'; // Ending the string

        } // If said amounts don't match there was a file stream fail
        else {
            *error = _FILE_STREAM_FAILED;
            buffer = NULL;
        }

    }// Memory allocation wasn't successful
    else {
        *error = _LACK_OF_MEMORY;
    }

    return buffer;
}



char* rle_block_decompressor (char* buffer, unsigned long block_size, long long* size_string, _modules_error* error) 
{
    // Assumption of the smallest size possible for the decompressed file
    int orig_size;
    if (block_size <= _64KiB) 
        orig_size = _64KiB + _1KiB;
    else if (block_size <= _640KiB) 
        orig_size = _640KiB + _1KiB;
    else if (block_size <= _8MiB)
        orig_size = _8MiB + _1KiB;
    else 
        orig_size = _64MiB + _1KiB;

    // Allocation of the corresponding memory 
    char* sequence = malloc(sizeof(char)*orig_size + 1);
    if (sequence) {

        // Loop to decompress block by block
        int l = 0;
        for (int i = 0; i < block_size; ++i) {
            
            char simb = buffer[i];
            int n_reps = 0;
            // Case of RLE pattern {0}char{n_rep} 
            if (!simb) {
                simb = buffer[++i];
                n_reps = buffer[++i];
            } 
            // Re-allocation of memory in string
            if (l + n_reps > orig_size) {
                switch (orig_size) {
                    case _64KiB + _1KiB:
                        orig_size = _640KiB + _1KiB;
                        break;
                    case _640KiB + _1KiB:
                        orig_size = _8MiB + _1KiB;
                        break;
                    case _8MiB + _1KiB:
                        orig_size = _64MiB + _1KiB;
                        break;
                    default:
                        *error = _FILE_UNRECOGNIZABLE;
                        free(sequence);
                        return NULL;
                }

                sequence = realloc(sequence, orig_size);
                if (!sequence) {
                    *error = _LACK_OF_MEMORY;
                    free(sequence);
                    break;
                }

            }
            if (n_reps) {
                memset(sequence + l, simb, n_reps);
                l += n_reps;
            }
            else {
                sequence[l++] = simb;
            }

        }
        *size_string = l;
        sequence[++l] = '\0';

    }
    else {
        *error = _LACK_OF_MEMORY;
    }

    return sequence;
}

_modules_error rle_decompress (char ** const path, const BlocksSize* blocks_size)  
{
    _modules_error error = _SUCCESS;
    // Opening RLE file
    FILE* f_rle = fopen(*path, "rb");
    if (f_rle) {
        // Creating path to FREQ file
        char* path_freq = add_ext(*path, FREQ_EXT);
        if (path_freq) {
            // Opening FREQ file
            FILE* f_freq = fopen(path_freq, "rb");
            if (f_freq) {
                // Creating path to TXT file
                char* path_txt = rm_ext(*path);
                if (path_txt) {
                    // Opening TXT file
                    FILE* f_txt = fopen(path_txt, "wb");
                    if (f_txt) {
                        // Loop to execute block by block
                        for (long long i = 0; i < blocks_size->length; ++i) {
                            // Loading rle block
                            char* buffer = load_rle(f_rle, blocks_size->sizes[i], &error);
                            if (error == _SUCCESS) {
                                // Decompressing the RLE block
                                long long size_sequence;
                                char* sequence = rle_block_decompressor(buffer, blocks_size->sizes[i], &size_sequence, &error);
                                if (error == _SUCCESS) {
                                    // Writing the decompressed block in TXT file
                                    if (fwrite(sequence, 1, size_sequence, f_txt) != size_sequence) {
                                        error = _FILE_STREAM_FAILED;
                                        i = blocks_size->length;
                                    }

                                    free(sequence);

                                }
                                else {
                                    i = blocks_size->length;
                                }
                                free(buffer);

                            }
                            else {
                                i = blocks_size->length;
                            }

                        }
                        
                        fclose(f_txt);
                    }
                    else {
                        error = _FILE_INACCESSIBLE;
                    }
                    free(path_txt);

                }
                else {
                    error = _LACK_OF_MEMORY;
                }
                fclose(f_freq);

            }
            else {
                error = _FILE_INACCESSIBLE;
            }
            free(path_freq);

        }
        else {
            error = _LACK_OF_MEMORY;
        }

        fclose(f_rle);

    }
    else {
        error = _FILE_INACCESSIBLE;
    }

    return error;
}



typedef struct btree{
    char symbol;
    struct btree *left,*right;
} *BTree;




void free_tree(BTree tree) {

    if (tree) {
        free_tree(tree->right);
        free_tree(tree->left);
        free(tree);
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

_modules_error create_tree (FILE* f_cod, unsigned long* block_sizes, long long index, BTree* decoder)
{
    _modules_error error = _SUCCESS;
    // Initialize root without meaning 
    *decoder = malloc(sizeof(struct btree));
   
    if (*decoder) {
        
        (*decoder)->left = (*decoder)->right = NULL;
       
        unsigned long crrb_size; 
        // Reads the current block size
        if (fscanf(f_cod, "@%lu", &crrb_size) == 1) {
           
            // Saves it to the array
            block_sizes[index] = crrb_size; 
            // Allocates memory to a block of code from .cod file
            char* code = malloc(33152);            
            if (code) {
                // Places it in the allocated string
                if (fscanf(f_cod,"@%33151[^@]", code) == 1) {   
                         
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
        
    }
    else {
        error = _LACK_OF_MEMORY;
    }

    
    return error;
}


char* shafa_block_decompressor (char* shafa, unsigned long shafa_size, unsigned long* blocks_size, long long index, BTree decoder) {
    
    char* decomp = malloc(blocks_size[index]);
    if (!decomp) return NULL;
    BTree root = decoder;
    uint8_t mask = 128; // 1000 0000 
    long long l = 0;
    int bit;
    for (int i = 0; i < shafa_size;) {
        
        bit = mask & shafa[i];
        if (!bit) decoder = decoder->left;
        else decoder = decoder->right;
        // Finds a leaf of the tree
        if (decoder && !(decoder->left) && !(decoder->right)) {
                decomp[l++] = decoder->symbol;
                decoder = root;
        }
        mask >>= 1; // 1000 0000 >> 0100 0000 >> ... >> 0000 0001 >> 0000 0000
        
        if (!mask) {
            ++i;
            mask = 128;
        }
        
    }
    decomp[l] = '\0';
    return decomp;
}


_modules_error shafa_decompress (char ** const path, BlocksSize * blocks_size) {

    _modules_error error = _SUCCESS;

    FILE* f_shafa = fopen(*path, "rb");
    if (f_shafa) {

        char* path_cod = rm_ext(*path);
        if (path_cod) {

            path_cod = add_ext(path_cod, CODES_EXT);
            if (path_cod) {

                FILE* f_cod = fopen(path_cod, "rb");
                if (f_cod) {

                    char* path_wrt = rm_ext(*path);
                    if (path_wrt) {

                        FILE* f_wrt = fopen(path_wrt, "wb");
                        if (f_wrt) {

                            if (fscanf(f_shafa, "@%lld", &blocks_size->length) == 1) {

                                char mode;
                                if (fscanf(f_cod, "@%c@%lld", &mode, &blocks_size->length)) {
                                    
                                    if ((mode == 'N') || (mode == 'R')) { // TROCAR O IF DE BAIXO COM O DE CIMA
                                        
                                        blocks_size->sizes = malloc(sizeof(int)*blocks_size->length);
                                        if (blocks_size->sizes) {
                                       
                                            for (long long i = 0; i < blocks_size->length; ++i) {

                                                BTree decoder;
                                                error = create_tree(f_cod, blocks_size->sizes, i, &decoder);

                                                if (error == _SUCCESS) {

                                                    unsigned long sf_bsize;
                                                    if (fscanf(f_shafa, "@%lu@", &sf_bsize) == 1) {

                                                        char* shafa_code = malloc(sf_bsize);
                                                        if (shafa_code) {

                                                            if (fread(shafa_code, 1, sf_bsize, f_shafa) == sf_bsize) { 
                                                                
                                                                char* decomp = shafa_block_decompressor(shafa_code, sf_bsize, blocks_size->sizes , i , decoder);

                                                                if (decomp) {

                                                                    if (fwrite(decomp, 1, blocks_size->sizes[i], f_wrt) != blocks_size->sizes[i]) {
                                                                        error = _FILE_STREAM_FAILED;
                                                                        i = blocks_size->length;
                                                                    }
                                                                    free(decomp);
                                                                }
                                                                else {
                                                                    error = _LACK_OF_MEMORY;
                                                                    i = blocks_size->length;
                                                                }
                                                            }
                                                            else {
                                                                error = _FILE_STREAM_FAILED;
                                                                i = blocks_size->length;
                                                            }

                                                               

                                                            free(shafa_code);
                                                        }
                                                        else {
                                                            error = _LACK_OF_MEMORY;
                                                            i = blocks_size->length;
                                                        }

                                                    }
                                                    else {
                                                        error = _FILE_STREAM_FAILED;
                                                    }

                                                free_tree(decoder);
                                                }
                                                else {
                                                    i = blocks_size->length;
                                                }
                                            }
                                        }
                                        else {
                                            error = _LACK_OF_MEMORY;
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
