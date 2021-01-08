/***************************************************
 *
 *  Author(s): Alexandre Martins, Beatriz Rodrigues
 *  Created Date: 3 Dec 2020
 *  Updated Date: 3 Jan 2020
 *
 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


#include "utils/file.h"
#include "utils/errors.h"
#include "utils/extensions.h"
#include "utils/multithread.h"

#define NUM_SYMBOLS 256

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
static inline void print_summary (double time, unsigned long * decomp_sizes, unsigned long * new_sizes, unsigned long long length, char* new_path, Algorithm algo) 
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

    for (unsigned long long i = 0; i < length; ++i) 
        printf("Size before/after generating file (block %lu): %lu/%lu\n", i + 1, decomp_sizes[i], new_sizes[i]);
    printf(
        "Module runtime (in milliseconds): %f\n"
        "Generated file %s\n", 
        time, new_path
    );
}


/**
\brief Loads the rle file to a buffer and saves it
 @param f_rle Pointer to the RLE file
 @param block_size Size of the block to be loaded
 @param buffer Address to a string to load a block of RLE content
 @returns Error status
*/
static _modules_error load_rle (FILE* f_rle, unsigned long block_size, uint8_t ** buffer) 
{
    _modules_error error = _SUCCESS;

    // Memory allocation for the buffer that will contain the contents of one block of symbols
    *buffer = malloc(block_size);
    if (*buffer) {
        // The function fread loads the said contents into the buffer
        // For the correct execution, the amount read by fread has to match the amount that was supposed to be read: block_size
        if (fread(*buffer, sizeof(uint8_t), block_size, f_rle) != block_size) {
            error = _FILE_STREAM_FAILED;
            *buffer = NULL; 
        }

    } 
    else 
        error = _LACK_OF_MEMORY;


    return error;
}

/**
 \brief Arguments to the RLE multithread
*/
typedef struct {

    FILE * f_rle;
    FILE * f_wrt;
    unsigned long rle_block_size;
    unsigned long * final_sizes;
    uint8_t * buffer;
    uint8_t * sequence;
    
} ArgumentsRLE;

/**
\brief Decompresses a RLE block
 @param args Arguments necessary to the function
 @returns Error status 
*/
static _modules_error rle_block_decompressor (void * _args) 
{
    _modules_error error = _SUCCESS;
    ArgumentsRLE * args = (ArgumentsRLE *) _args;  
    unsigned long block_size = args->rle_block_size;
    unsigned long * final_sizes = args->final_sizes;
    uint8_t * buffer = args->buffer;
    uint8_t * sequence;
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
    sequence = malloc(orig_size);
    if (sequence) {

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
                        free(sequence);
                        return error;
                }
                sequence = realloc(sequence, orig_size);
                // In case of realloc failure, we free the previous memory allocation
                if (!sequence) {
                    error = _LACK_OF_MEMORY;
                    free(sequence); // Frees the previously allocated memory
                    break;
                }

            }
            if (n_reps) {
                memset(sequence + l, simb, n_reps);
                l += n_reps;
            }
            else 
                sequence[l++] = simb;
            
        }
        *final_sizes = l; 

        args->sequence = sequence;
    }
    else 
        error = _LACK_OF_MEMORY;
    
    free(buffer);

    return error;
}

/**
 \brief Writes the contents resulting of the decompression of the RLE file
 @param _args Arguments to the function
 @param prev_error Error status in previous thread
 @param error Error status from process
 @returns Error status
*/
static _modules_error write_decompressed_rle (void * const _args, _modules_error prev_error, _modules_error error)
{
    ArgumentsRLE * args = (ArgumentsRLE *) _args;
    FILE * const f_wrt = args->f_wrt;
    const unsigned long new_block_size = *args->final_sizes;
    uint8_t * const sequence = args->sequence;

    if (!error) {

        if (!prev_error) {

            // Writing the decompressed block in ORIGINAL file
            if (fwrite(sequence, sizeof(uint8_t), new_block_size, f_wrt) != new_block_size) 
                error = _FILE_STREAM_FAILED; 

        }

        free(sequence); 
    }

    free(_args);

    return error;
}


_modules_error rle_decompress (char ** path) 
{
    _modules_error error = _SUCCESS;
    FILE *f_rle, *f_freq, *f_wrt;
    char *path_freq, *path_wrt, *path_rle;
    uint8_t * buffer;
    char mode;
    unsigned long *rle_sizes, *final_sizes;
    unsigned long long length;
    float total_time;
    ArgumentsRLE * args;
    
    clock_main_thread(START_CLOCK);

    path_rle = *path;
    // Opens RLE file
    f_rle = fopen(path_rle, "rb");
    if (f_rle) {

        // Creates path to the original file
        path_wrt = rm_ext(path_rle);
        if (path_wrt) {

            // Opens file to write the original contents in
            f_wrt = fopen(path_wrt, "wb");
            if (f_wrt) {
                
                // Creates path to the FREQ file
                path_freq = add_ext(path_rle, FREQ_EXT);
                if (path_freq) {

                    // Opens FREQ file
                    f_freq = fopen(path_freq, "rb");
                    if (f_freq) {

                        // Reads the header of the FREQ file
                        if (fscanf(f_freq, "@%c@%lu", &mode, &length) == 2) {   

                            if (mode == 'R') {

                                // Allocates memory for an array to contain the sizes of all the blocks of the RLE file
                                rle_sizes = malloc(sizeof(unsigned long) * length);       
                                if (rle_sizes) {

                                    // Loads the sizes to the array
                                    for (unsigned long long i = 0; i < length && !error; ++i) {
                                        if (fscanf(f_freq, "@%lu@%*[^@]", rle_sizes + i) != 1)                                          
                                            error = _FILE_STREAM_FAILED;       
                                                                                                                   
                                    }

                                    if (error) 
                                        free(rle_sizes);

                                }   
                                else 
                                    error = _LACK_OF_MEMORY;                      

                            }
                            else 
                                error = _FILE_UNRECOGNIZABLE;

                        }
                        else
                            error = _FILE_STREAM_FAILED;  
                            

                        fclose(f_freq);                   

                    }
                    else 
                        error = _FILE_INACCESSIBLE;

                    free(path_freq);

                }   
                else 
                    error = _LACK_OF_MEMORY;  

                // If there wasn't an error in any of the previous processes, the decompression continues
                if (!error) {

                    // Allocates memory for an array that will contain the final size of the blocks after decompression
                    final_sizes = malloc(sizeof(unsigned long) * length);
                    if (final_sizes) {

                        // Loop to execute block by block
                        for (unsigned long long thread_idx = 0; thread_idx < length; ++thread_idx) {
                                
                            // Loading rle block
                            error = load_rle(f_rle, rle_sizes[thread_idx], &buffer);
                            if (error) break;

                            args = malloc(sizeof(ArgumentsRLE)); 

                            if (!args) {
                                free(buffer);
                                break;
                            }

                            *args = (ArgumentsRLE) {
                                .rle_block_size = rle_sizes[thread_idx],
                                .buffer = buffer,
                                .f_rle = f_rle, 
                                .f_wrt = f_wrt,
                                .final_sizes = &final_sizes[thread_idx]

                            };       

                            // Decompressing the RLE block and loading the final size of the blocks after decompression to the array
                            error = multithread_create(rle_block_decompressor, write_decompressed_rle, args);
                                
                            if (error) {
                                free(args);
                                free(buffer);
                                break;
                            }
    
                        }

                        multithread_wait();

                                         
                        if (error) 
                            free(final_sizes);                  
                    }
                    else 
                        error = _LACK_OF_MEMORY;
                }
                fclose(f_wrt);
                    
            }
            else 
                error = _FILE_INACCESSIBLE;

        }
        else 
            error = _LACK_OF_MEMORY;
        
        fclose(f_rle);
    }
    else 
        error = _FILE_INACCESSIBLE;

    if (!error) {
        
        free(path_rle);
        *path = path_wrt;
        total_time = clock_main_thread(STOP_CLOCK);
        print_summary(total_time, rle_sizes, final_sizes, length, *path, _RLE);
        free(rle_sizes);
        free(final_sizes);

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
    for (int i = 0; code[i]; ++i) {

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

/*
Struct for the SHAFA arguments in multithreading
*/
typedef struct {

	FILE * f_wrt;
    char * cod_code;
	unsigned long * rle_sizes;
	unsigned long * final_sizes;
	uint8_t * rle_decompressed;
	uint8_t * shafa_decompressed;
	uint8_t * shafa_code;
    bool rle_decompression;
		
} ArgumentsSHAFA;

/**
\brief Generates a binary tree that contains the symbols acording to the codes
 @param code String with a block of the COD file
 @param decoder Pointer to the binary tree
 @returns Error status
*/
static _modules_error create_tree (char * code, BTree * decoder)
{
    _modules_error error;
    char *sf;
    unsigned long j;

    error = _SUCCESS;
    // Initialize root without meaning 
    *decoder = malloc(sizeof(struct btree));
    
    if (*decoder) {
        
        (*decoder)->left = (*decoder)->right = NULL;
          
        for (unsigned long k = 0, l = 0; code[l] && !error;) {

            // When it finds a ';' it is no longer on the same symbol. This updates it.
            while (code[l] == ';') {
                k++;
                l++;
            }
            // Allocates memory for the sf code of a symbol
            sf = malloc(NUM_SYMBOLS + 1); // 256 maximum + 1 NULL
            if (sf) {      
                for (j = 0; code[l] && (code[l] != ';'); ++j, ++l) 
                    sf[j] = code[l];
                sf[j] = '\0';           
                if (j != 0) 
                    // Adds the code to the tree
                    error = add_tree(decoder, sf, k);
                                
                             
                free(sf);        
            }
            else  
                error = _LACK_OF_MEMORY;                
        }

        free(code);       
    }
    else 
        error = _LACK_OF_MEMORY;


    return error;
}

/**
\brief Decompresses a block of shafa code
 @param shafa Content of the file to be descompressed
 @param block_size Block size
 @param decoder Binary tree with the symbols
 @param decomp Address to load a string with the decompressed contents
 @returns Error status
*/
static _modules_error shafa_block_decompressor (uint8_t * shafa, unsigned long block_size, BTree decoder, uint8_t ** decomp) 
{
    BTree root;
    uint8_t mask;
    unsigned long i, l;
    int bit;

    // String for the decompressed contents 
    *decomp = malloc(block_size);
    if (!(*decomp)) return _LACK_OF_MEMORY;

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
                (*decomp)[l++] = decoder->symbol;
                decoder = root;
        }
        mask >>= 1; // 1000 0000 >> 0100 0000 >> ... >> 0000 0001 >> 0000 0000
        
        // When mask = 0 it's time to move to the next byte
        if (!mask) {
            ++i;
            mask = 128;
        }
        
    }
      
    return _SUCCESS;
}

/** Does the process of the main function: includes the creation of a binary tree, the shafa block decompression and, if needed, the rle block decompression
 \brief 
 @param _args Arguments of the function
 @returns Error status
*/
static _modules_error process_shafa_decomp (void * _args) {

    _modules_error error;
    ArgumentsSHAFA * args_shafa = (ArgumentsSHAFA *) _args; 
    BTree decoder; 
    ArgumentsRLE args_rle;

    error = create_tree(args_shafa->cod_code, &decoder);

    if (!error) {

        error = shafa_block_decompressor(args_shafa->shafa_code, *args_shafa->rle_sizes, decoder, &args_shafa->shafa_decompressed);

        free(args_shafa->shafa_code);
        free_tree(decoder);

        if (!error && args_shafa->rle_decompression) {

            args_rle = (ArgumentsRLE) {
                .buffer = args_shafa->shafa_decompressed,
                .rle_block_size = *args_shafa->rle_sizes,
                .final_sizes = args_shafa->final_sizes
            };

            error = rle_block_decompressor(&args_rle);
            if (!error) {
                args_shafa->rle_decompressed = args_rle.sequence;
            }
        }
    }

    return error;
}

/**
 \brief Writes the decompressed shafa in the destined file
 @param _args Arguments of the function
 @param prev_error Previous thread error status
 @param error Process error status
 @returns Error status
*/
static _modules_error write_decompressed_shafa (void * _args, _modules_error prev_error, _modules_error error) {

    ArgumentsSHAFA * args_shafa = (ArgumentsSHAFA *) _args; 
    unsigned long size_wrt;
    uint8_t * decomp;
    FILE * f_wrt = args_shafa->f_wrt;
    bool rle_decompression = args_shafa->rle_decompression;

    if (!error) {

        if (!prev_error) {

            size_wrt = (rle_decompression) ? (*args_shafa->final_sizes) : (*args_shafa->rle_sizes);
            decomp = (rle_decompression) ? (args_shafa->rle_decompressed) : (args_shafa->shafa_decompressed);
            if (fwrite(decomp, sizeof(uint8_t), size_wrt, f_wrt) != size_wrt) 
                error = _FILE_STREAM_FAILED;

        }

        if (rle_decompression) 
            free(args_shafa->rle_decompressed);    
    } 

    free(_args);

    return error;
}


_modules_error shafa_decompress (char ** const path, bool rle_decompression) 
{
    _modules_error error;
    FILE *f_shafa, *f_cod, *f_wrt;
    char *path_cod, *path_wrt, *path_shafa, *path_tmp;
    uint8_t * shafa_code; 
    char * cod_code;
    char mode;
    float total_time;
    unsigned long long length;
    unsigned long *sizes, *sf_sizes, *final_sizes;
    unsigned long sf_bsize;
    ArgumentsSHAFA * args;

    sizes = sf_sizes = final_sizes = NULL;
    path_shafa = *path;
    error = _SUCCESS;
    clock_main_thread(START_CLOCK);

    // Opening the shafa file
    f_shafa = fopen(path_shafa, "rb");
    if (f_shafa) {

        // Creates path to the .cod file
        path_tmp = rm_ext(path_shafa);
        if (path_tmp) { // free this somehow

            if (rle_decompression) {
                path_wrt = rm_ext(path_tmp);
                if (!path_wrt) 
                    error = _LACK_OF_MEMORY;
            }
            else 
                path_wrt = path_tmp;

            f_wrt = fopen(path_wrt, "wb");
            if (f_wrt) {
                
                path_cod = add_ext(path_tmp, CODES_EXT);
                if (path_cod) {

                    f_cod = fopen(path_cod, "rb");
                    if (f_cod) {

                        // Reading header of shafa file
                        if (fscanf(f_shafa, "@%lu", &length) == 1) {

                            // Reading header of cod file
                            if (fscanf(f_cod, "@%c@%lu", &mode, &length) == 2) {
                                // Checking the mode of the file
                                if ((mode == 'N' && !rle_decompression) || (mode == 'R')) {   

                                    // Allocates memory to an array with the purpose of saving the size of each SHAF block
                                    sf_sizes = malloc(sizeof(unsigned long) * length);
                                    if (sf_sizes) {

                                        // Allocates memory to an array with the purpose of saving the sizes of each RLE/ORIGINAL block
                                        sizes = malloc(sizeof(unsigned long) * length);
                                        if (sizes) {

                                            if (rle_decompression) {
                                                final_sizes = malloc(sizeof(unsigned long) * length);
                                                if (!final_sizes)
                                                    error = _LACK_OF_MEMORY;
                                            }  

                                            for (unsigned long long thread_idx = 0; thread_idx < length && !error; ++thread_idx) {

                                                // Reads the size of the shafa blockss
                                                if (fscanf(f_shafa, "@%lu@", &sf_bsize) == 1) {

                                                    sf_sizes[thread_idx] = sf_bsize;

                                                    // Allocates memory to a buffer in which will be loaded one block of shafa code                                                         
                                                    shafa_code = malloc(sf_bsize); 
                                                    if (shafa_code) {

                                                        // Reads a block of shafa code
                                                        if (fread(shafa_code, sizeof(uint8_t), sf_bsize, f_shafa) == sf_bsize) { 

                                                            // Reads the size of the decompressed shafa code and saves it
                                                            if (fscanf(f_cod, "@%lu", &sizes[thread_idx]) == 1) {

                                                                // Allocates memory for a block of COD code
                                                                cod_code = malloc(33152); //sum 1 to 256 (worst case shannon fano) + 255 semicolons + 1 byte NULL
                                                                if (cod_code) {

                                                                    // Loads the block of COD code
                                                                    if (fscanf(f_cod,"@%33151[^@]", cod_code) == 1) {

                                                                        // Allocates memory for the arguments
                                                                        args = malloc(sizeof(ArgumentsSHAFA)); 
                                                                        if (!args) {
                                                                            error = _LACK_OF_MEMORY;
                                                                            free(shafa_code);
                                                                            break;
                                                                        }
                                                                            
                                                                        // Arguments for the SHAFA multithread
                                                                        *args = (ArgumentsSHAFA) {
                                                                            .f_wrt = f_wrt,
                                                                            .shafa_code = shafa_code,
                                                                            .rle_decompression = rle_decompression,
                                                                            .rle_sizes = &sizes[thread_idx],
                                                                            .final_sizes = &final_sizes[thread_idx],
                                                                            .cod_code = cod_code
                                                                        };
                                                                        error = multithread_create(process_shafa_decomp, write_decompressed_shafa, args); 
                                                                            
                                                                        if (error) {
                                                                            free(cod_code);
                                                                            break;
                                                                        }
                                                                    }
                                                                    else 
                                                                        error = _FILE_STREAM_FAILED;
                                                                            
                                                                }
                                                                else 
                                                                    error = _LACK_OF_MEMORY;
                                                            }
                                                            else 
                                                                error = _FILE_STREAM_FAILED;
                                                                   
                                                        }
                                                        else 
                                                            error = _FILE_STREAM_FAILED;
                                                 
                                                    }
                                                    else 
                                                        error = _LACK_OF_MEMORY;                                                      
                                                }
                                                else 
                                                    error = _FILE_STREAM_FAILED;

                                                } 
                                                multithread_wait();

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
                                error = _FILE_STREAM_FAILED;
                        }
                        else 
                            error = _FILE_STREAM_FAILED;

                        fclose(f_cod);
                        
                    }
                    else 
                        error = _FILE_INACCESSIBLE;
                    
                    free(path_cod);

                }
                else 
                    error = _LACK_OF_MEMORY;

                fclose(f_wrt);

            }
            else 
                error = _FILE_INACCESSIBLE;
            
            if (rle_decompression) 
                free(path_tmp);
        }
        else 
            error = _LACK_OF_MEMORY;

        fclose(f_shafa);
    }
    else 
        error = _FILE_INACCESSIBLE;

    if (!error) {
        total_time = clock_main_thread(STOP_CLOCK);                                
        *path = path_wrt;
        free(path_shafa);

        if (rle_decompression) {

            print_summary(total_time, sf_sizes, final_sizes, length, path_wrt, _SHAFA_RLE); 
            free(final_sizes);

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
