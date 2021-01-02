/***************************************************
 *
 *  Author(s): Alexandre Martins, Beatriz Rodrigues
 *  Created Date: 3 Dec 2020
 *  Updated Date: 2 Jan 2020
 *
 **************************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "d.h"
#include "utils/file.h"
#include "utils/extensions.h"

#define n_simb 256

/**
\brief Struct with the types of decoding possible
*/
typedef enum 
{
    _RLE,
    _SHAFA,
    _SHAFA_RLE,

} Algorithm;


/**
\brief Prints the results of the program execution
 @param time Time that the program took to execute
 @param decomp_sizes Block sizes previous to the decompression
 @param new_sizes Block sizes after the decompression
 @param length Number of blocks
 @param new_path The path to the generated file
 @param algo The type of decoding that occured
*/
static inline void print_summary (double time, unsigned long * decomp_sizes, unsigned long * new_sizes, long long length, char* new_path, Algorithm algo) 
{
    printf(
        "Alexandre Martins, a93242, MIEI/CD, 1-JAN-2021\n"
        "Beatriz Rodrigues, a93230, MIEI/CD, 1-JAN-2021\n"
    );

    if (algo == _RLE) 
        printf("Module: D (RLE decoding)\n");
    else if (algo == _SHAFA) 
        printf("Module: D (SHAFA decoding)\n");
    else 
        printf("Module: D (SHAFA & RLE decoding)\n");

    for (long long i = 0; i < length; ++i) 
        printf("Size before/after generating file (block %lld): %lu/%lu\n", i + 1, decomp_sizes[i], new_sizes[i]);
    printf(
        "Module runtime (in milliseconds): %f\n"
        "Generated file %s\n", 
        time, new_path
    );
}


/**
\brief Loads the rle file to a buffer and saves it
 @param f_rle Pointer to the RLE file
 @param size_of_block Size of the block to be loaded
 @param buffer Address to a string to load a block of RLE content
 @returns Error status
*/
static _modules_error load_rle (FILE* f_rle, unsigned long size_of_block, char ** buffer) 
{
    _modules_error error = _SUCCESS;

    // Memory allocation for the buffer that will contain the contents of one block of symbols
    *buffer = malloc(size_of_block + 1);

    if (*buffer) {
        // The function fread loads the said contents into the buffer
        // For the correct execution, the amount read by fread has to match the amount that was supposed to be read: size_of_block
        if (fread(*buffer,1,size_of_block, f_rle) == size_of_block) {

            (*buffer)[size_of_block] = '\0'; // Ending the string

        } // Error caused by said amount not matching
        else {
            error = _FILE_STREAM_FAILED;
            *buffer = NULL;
        }

    } 
    else {
        error = _LACK_OF_MEMORY;
    }

    return error;
}


/**
\brief Decompresses a RLE block
 @param buffer String with a block of the RLE file loaded
 @param block_size Size of the RLE block
 @param size_string Size of the decompressed block
 @param final_sizes Pointer to the address of an array with the purpose of saving the size of the decompressed block
 @param sequence Address to a string to hold the decompressed contents
 @returns Error status 
*/
static _modules_error rle_block_decompressor (char* buffer, unsigned long block_size, unsigned long *final_sizes, char ** sequence) 
{
    _modules_error error = _SUCCESS;
    unsigned long orig_size, l;
    char simb;
    uint8_t n_reps;

    // Assumption of the smallest size possible for the decompressed file
    if (block_size <= _64KiB) 
        orig_size = _64KiB + _1KiB;
    else if (block_size <= _640KiB) 
        orig_size = _640KiB + _1KiB;
    else if (block_size <= _8MiB)
        orig_size = _8MiB + _1KiB;
    else 
        orig_size = _64MiB + _1KiB;

    // Allocation of the corresponding memory 
    *sequence = malloc(orig_size + 1);
    if (*sequence) {

        // Loop to decompress block by block
        l = 0; // Variable to be used to go through the sequence string 
        for (unsigned long i = 0; i < block_size; ++i) {
            
            simb = buffer[i];
            n_reps = 0;
            // Case of RLE pattern {0}char{n_rep} 
            if (!simb) {
                simb = buffer[++i];
                n_reps = buffer[++i];
            } 
            // Re-allocation of memory in the string
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
                    default: // Invalid size
                        error = _FILE_UNRECOGNIZABLE;
                        free(*sequence);
                        return error;
                }
                *sequence = realloc(*sequence, orig_size);
                // In case of realloc failure, we free the previous memory allocation
                if (!(*sequence)) {
                    error = _LACK_OF_MEMORY;
                    free(sequence); // Frees the previously allocated memory
                    sequence = NULL;
                    break;
                }

            }
            if (n_reps) {
                memset(*sequence + l, simb, n_reps);
                l += n_reps;
            }
            else {
                (*sequence)[l++] = simb;
            }

        }
        *final_sizes = l; 
        (*sequence)[++l] = '\0';

    }
    else {
        error = _LACK_OF_MEMORY;
    }

    return error;
}

/**
\brief Struct to save the number of blocks and the sizes of each one
*/
typedef struct {
    unsigned long * sizes;
    long long length;
} BlocksSize;


/**
\brief Decompresses an RLE file
 @param path Pointer to the RLE file's path
 @param blocks_size Block size
 @param final_sizes String size
 @returns Error status
*/
static _modules_error _rle_decompress (char ** const path, BlocksSize *blocks_size, BlocksSize *final_sizes) 
{
    _modules_error error = _SUCCESS; 
    FILE *f_rle, *f_freq, *f_wrt;
    char *path_freq, *path_wrt, *path_rle;
    char *buffer, *sequence;
    char mode;
    unsigned long *sizes;
    long long length;

    path_rle = *path;

    f_rle = fopen(path_rle, "rb");
    if (f_rle) {

        // Creates path to the original file
        path_wrt = rm_ext(path_rle);
        if (path_wrt) {

            f_wrt = fopen(path_wrt, "wb");
            if (f_wrt) {

                // If we don't have data from the RLE block sizes from the COD file we read them from the FREQ file
                if (!blocks_size->length) {
                    
                    //Creates path to the FREQ file
                    path_freq = add_ext(path_wrt, FREQ_EXT);
                    if (path_freq) {
                        
                        f_freq = fopen(path_freq, "rb");
                        if (f_freq) {
                            
                            // Reads the header of the FREQ file
                            if (fscanf(f_freq, "@%c@%lld", &mode, &length) == 2) {
                                // Checks if the format is correct
                                if ((mode == 'N') || (mode == 'R')) {

                                    // Allocates memory for an array to contain the sizes of all the blocks of the RLE file
                                    sizes = malloc(sizeof(unsigned long) * length);
                                    if (sizes) {

                                        for (long long i = 0; i < length && !error; ++i) {
                                            if (fscanf(f_freq, "@%lu@%*[^@]", sizes + i) != 1) {
                                                
                                                error = _FILE_STREAM_FAILED;
                                                
                                            }
                                        }
                                        
                                        if (error) {
                                            free(sizes);
                                        }
                                        else {
                                            blocks_size->sizes = sizes;
                                            blocks_size->length = length;
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
                }// If we have the sizes saved from the COD file we don't have to re-read them from the FREQ file; we just access the struct
                else {
                    sizes = blocks_size->sizes;
                    length = blocks_size->length;
                }

                // If there wasn't an error in any of the previous processes, the decompression continues
                if (!error) {
                    // Allocates memory for an array that will contain the final size of the blocks after decompression
                    final_sizes->sizes = malloc(sizeof(unsigned long) * length);
                    if (final_sizes->sizes) {
                        // Loop to execute block by block
                        for (long long i = 0; i < length && !error; ++i) {
                            buffer = sequence = NULL;
                            // Loading rle block
                            error = load_rle(f_rle, sizes[i], &buffer);
                            if (!error) {

                                // Decompressing the RLE block and loading the final size of the blocks after decompression to the array
                                error = rle_block_decompressor(buffer, sizes[i], final_sizes->sizes + i, &sequence); 
                                //sequence = rle_block_decompressor(buffer, sizes[i], &size_sequence, final_sizes->sizes + i, &error);
                                if (!error) {
                                    // Writing the decompressed block in ORIGINAL file
                                    if (fwrite(sequence, 1, final_sizes->sizes[i], f_wrt) != final_sizes->sizes[i]) {

                                        error = _FILE_STREAM_FAILED;

                                    }
                                }
                                free(sequence);

                            }
                            else {
                                error = _LACK_OF_MEMORY;
                            }
                            free(buffer);
                        }
                        if (error) {
                            free(final_sizes->sizes); 
                        }                   
                    }
                    else {

                        error = _LACK_OF_MEMORY;
                    }

                    fclose(f_wrt);
                }

            }
            else {
                error = _FILE_INACCESSIBLE;
            }
            if (error)
                free(path_wrt);
        }
        else {
            error = _LACK_OF_MEMORY;
        }
        fclose(f_rle);
       
    }
    else {
        error = _FILE_INACCESSIBLE;
    }

    if (!error) {
        
        free(path_rle);
        *path = path_wrt;
        
    }

    return error;
}


_modules_error rle_decompress(char ** path) 
{
    clock_t t;
    _modules_error error = _SUCCESS;
    double total_time;
    BlocksSize before, after; // The before saves the RLE block sizes, the after saves the decompressed sizes

    t = clock();
    before = after = (BlocksSize) {.sizes = NULL, .length = 0};

    // Decompresses the RLE file and loads the RLE blocks sizes and the decompressed sizes
    error = _rle_decompress(path, &before, &after);
    if (!error) {

        t = clock() - t;
        total_time = (((double) t) / CLOCKS_PER_SEC) * 1000;
        print_summary(total_time, before.sizes, after.sizes, before.length, *path, _RLE);
        free(before.sizes);
        free(after.sizes);

    }

    return error;
}

/**
\brief Binary tree that will contain all of the symbols codes needed to the descompressed file
*/
typedef struct btree{
    char symbol;
    struct btree *left,*right;
} *BTree;

/**
\brief Frees all the memory used by a BTree
 @param tree Binary tree
*/
static void free_tree(BTree tree) 
{
    if (tree) {
        free_tree(tree->right);
        free_tree(tree->left);
        free(tree);
    }

}

/**
\brief Adds a given symbol to the BTree
 @param decoder Tree with saved symbols to help in decoding
 @param code Path to save the symbol in the correct leaf
 @param symbol Symbol to be saved in the tree 
 @returns Error status
*/
static _modules_error add_tree(BTree* decoder, char *code, char symbol) 
{
    // Creation of the path to the symbol we are placing
    for (unsigned long i = 0; code[i]; ++i) {

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
    // Adding the symbol to the corresponding leaf of the tree
    *decoder = malloc(sizeof(struct btree));
    (*decoder)->symbol = symbol;
    (*decoder)->left = (*decoder)->right = NULL;
    return _SUCCESS;
}

/**
\brief Generates a binary tree that contains the symbols acording to the codes
 @param f_cod File .cod
 @param block_sizes Block sizes
 @param index Index to the block that we are currently on
 @param decoder Pointer to the binary tree
 @returns Error status
*/
static _modules_error create_tree (FILE* f_cod, unsigned long* block_sizes, long long index, BTree* decoder)
{
    _modules_error error;
    unsigned long crrb_size;
    char *code, *sf;
    unsigned long j;

    error = _SUCCESS;
    // Initialize root without meaning 
    *decoder = malloc(sizeof(struct btree));
    
    if (*decoder) {
        
        (*decoder)->left = (*decoder)->right = NULL;
       
        // Reads the current block size
        if (fscanf(f_cod, "@%lu", &crrb_size) == 1) {
           
            // Saves it to the array
            block_sizes[index] = crrb_size; 
            // Allocates memory to a block of code from .cod file
            code = malloc(33152); //sum 1 to 256 (worst case shannon fano) + 255 semicolons + 1 byte NULL + 1 extra byte for algorithm efficiency avoiding 256 compares           
            if (code) {
                
                if (fscanf(f_cod,"@%33151[^@]", code) == 1) {   
                         
                    for (unsigned long k = 0, l = 0; code[l] && !error;) {
                        // When it finds a ';' it is no longer on the same symbol. This updates it.
                        while (code[l] == ';') {
                            k++;
                            l++;
                        }
                        // Allocates memory for the sf code of a symbol
                        sf = malloc(257); // 256 maximum + 1 NULL
                        if (sf) {      
                            for (j = 0; code[l] && (code[l] != ';'); ++j, ++l) 
                                sf[j] = code[l];
                            sf[j] = '\0';           
                            if (j != 0) {
                                // Adds the code to the tree
                                error = add_tree(decoder, sf, k);
                                
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

/**
\brief Decompresses a block of shafa code
 @param shafa Content of the file to be descompressed
 @param block_size Block size
 @param decoder Binary tree with the symbols
 @returns String with the decompressed block
*/
static char* shafa_block_decompressor (char* shafa, unsigned long block_size, BTree decoder) 
{
    char* decomp;
    BTree root;
    uint8_t mask;
    unsigned long i, l;
    int bit;

    // String for the decompressed contents 
    decomp = malloc(block_size + 1);
    if (!decomp) return NULL;

    root = decoder; // Saving the root for multiple crossings in the tree
    mask = 128; // 1000 0000 
    i = l = 0; 

    // Loop to check every byte, bit by bit (it's used the final size to control the cycle to avoid padding excess)
    while (l < block_size) {
        
        bit = mask & shafa[i];
        if (!bit) decoder = decoder->left; // bit = 0
        else decoder = decoder->right;
        // Finds a leaf of the tree
        if (decoder && !(decoder->left) && !(decoder->right)) {
                decomp[l++] = decoder->symbol;
                decoder = root;
        }
        mask >>= 1; // 1000 0000 >> 0100 0000 >> ... >> 0000 0001 >> 0000 0000
        
        // When mask = 0 it's time to move to the next byte
        if (!mask) {
            ++i;
            mask = 128;
        }
        
    }
    
    decomp[l] = '\0';
    return decomp;
}


_modules_error shafa_decompress (char ** const path, bool rle_decompression) 
{
    _modules_error error;
    FILE *f_shafa, *f_cod, *f_wrt;
    char *path_cod, *path_wrt, *path_shafa;
    char *shafa_code, *decomp; 
    char mode;
    clock_t t;
    double total_time;
    long long length;
    unsigned long *sizes, *sf_sizes;
    unsigned long sf_bsize;
    BTree decoder;
    BlocksSize blocks_size, final_sizes;

    sizes = sf_sizes = NULL;
    path_shafa = *path;
    error = _SUCCESS;
    decoder = NULL;
    t = clock();

    // Opening the files
    f_shafa = fopen(path_shafa, "rb");

    if (f_shafa) {
        
        // Creates path to the .cod file
        path_cod = rm_ext(path_shafa);
        if (path_cod) {

            path_cod = add_ext(path_cod, CODES_EXT);
            if (path_cod) {

                f_cod = fopen(path_cod, "rb");
                if (f_cod) {

                    // Creates path to the original file
                    path_wrt = rm_ext(path_shafa);
                    if (path_wrt) {

                        f_wrt = fopen(path_wrt, "wb");
                        if (f_wrt) {

                            // Reading header of shafa file
                            if (fscanf(f_shafa, "@%lld", &length) == 1) {

                                // Reading header of cod file
                                if (fscanf(f_cod, "@%c@%lld", &mode, &length) == 2) {
                                    
                                    // Checking the mode of the file
                                    if ((mode == 'N' && !rle_decompression) || (mode == 'R')) { 

                                        // Allocates memory to an array with the purpose of saving the size of each SHAF block
                                        sf_sizes = malloc(sizeof(unsigned long)*length);
                                        if (sf_sizes) {

                                            // Allocates memory to an array with the purpose of saving the sizes of each RLE/ORIGINAL block
                                            sizes = malloc(sizeof(unsigned long)*length);
                                            if (sizes) {
                                                
                                                for (long long i = 0; i < length && !error; ++i) {

                                                    // Creates the binary tree with the paths to decode the symbols
                                                    error = create_tree(f_cod, sizes, i, &decoder);

                                                    if (!error) {

                                                        // Allocates memory to an array with the purpose of saving the sizes of each shafa block
                                                        if (fscanf(f_shafa, "@%lu@", &sf_bsize) == 1) {

                                                            sf_sizes[i] = sf_bsize; 
                                                            
                                                            // Allocates memory to a buffer in which will be loaded one block of shafa code                                                         
                                                            shafa_code = malloc(sf_bsize + 1);
                                                            if (shafa_code) {

                                                                if (fread(shafa_code, 1, sf_bsize, f_shafa) == sf_bsize) { 

                                                                    // Generates the block decoded to RLE/ORIGINAL
                                                                    decomp = shafa_block_decompressor(shafa_code, sizes[i], decoder);
                                                                    if (decomp) {
                                                                        
                                                                        // Writes the block in the destined file
                                                                        if (fwrite(decomp, 1, sizes[i], f_wrt) != sizes[i]) {
                                                                            error = _FILE_STREAM_FAILED;
                                                                        }
                                                                        free(decomp);
                                                                    }
                                                                    else {
                                                                        error = _LACK_OF_MEMORY;
                                                                    }
                                                                }
                                                                else {
                                                                    error = _FILE_STREAM_FAILED;
                                                                }
                                                                free(shafa_code);
                                                            }
                                                            else {
                                                                error = _LACK_OF_MEMORY;
                                                            }
                                                        }
                                                        else {
                                                            error = _FILE_STREAM_FAILED;
                                                        }

                                                        
                                                    free_tree(decoder);
                                                    decoder = NULL;
                                                    }
                                                }
                                            
                                            }
                                            else {
                                                error = _LACK_OF_MEMORY;
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
                        if (error) {
                            free(path_wrt);
                        }
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
        error = _FILE_INACCESSIBLE;
    }

    // If it's supposed to be RLE compressed as well                                            
    if (!error && rle_decompression) {   

       
        blocks_size.sizes = sizes; 
        blocks_size.length = length;
        final_sizes = (BlocksSize) {.sizes = NULL, .length = 0}; // Struct to save the final sizes of the blocks to print later 
        error = _rle_decompress(&path_wrt, &blocks_size, &final_sizes);                     
        
    }
    
    if (!error) {

        t = clock() - t;
        total_time = (((double) t) / CLOCKS_PER_SEC) * 1000;                                  
        *path = path_wrt;
        free(path_shafa);

        if (rle_decompression) {

            print_summary(total_time, sf_sizes, final_sizes.sizes, length, path_wrt, _SHAFA_RLE); 
            free(final_sizes.sizes);

        }// If RLE decompression didn't occur
        else {
            print_summary(total_time, sf_sizes, sizes, length, path_wrt, _SHAFA);                                               
        }
    }
    
                                           
    if (sizes) 
        free(sizes);
    if (sf_sizes) 
        free(sf_sizes);

    return error;
}
