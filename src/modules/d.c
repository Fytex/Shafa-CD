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
        *error = _FILE_CORRUPTED;
        return NULL;
    }
    buffer[res] = '\0';
    return buffer;
}

char* decompress_string (char* buffer, int block_size, int* size_sequence) 
{
    char* sequence = malloc(5000); // CORRIGIR: arranjar algo mais eficiente
    if (!sequence) return NULL;
    
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

_modules_error rle_decompress(char** const path) 
{
    // Opening the files
    FILE* f_rle = fopen(*path, "rb");
    if (!f_rle) return _FILE_INACCESSIBLE;
    FILE* f_freq = fopen(add_ext(*path, FREQ_EXT), "rb");
    if (!f_freq) return _FILE_INACCESSIBLE;
    FILE* f_txt = fopen(rm_ext(*path), "wb");
    if (!f_txt) return _FILE_INACCESSIBLE;
    
    // Reads header
    int n_blocks; 
    if (fscanf(f_freq, "@R@%d@", &n_blocks) != 1) return _FILE_UNRECOGNIZABLE;
    
    // Reads from RLE and FREQ , while writting the decompressed version of its contents in the TXT file
    for (int i = 0; i < n_blocks; ++i) {
        int block_size;
        if (fscanf(f_freq, "%d[^@]", &block_size) == 1) { // Reads the size of the block
            _modules_error error;
            char* buffer = load_rle(f_rle, block_size, &error); // Loads block to buffer
            if (!buffer) return error; // When buffer is NULL there was an error in load_rle that should be reported
            int size_sequence;
            char* sequence = decompress_string(buffer, block_size, &size_sequence);
            if (!sequence) return _LACK_OF_MEMORY;
            free(buffer);
            int res = fwrite(sequence, 1, size_sequence, f_txt); // Writes decompressed string in txt file
            if (res != size_sequence) return _FILE_CORRUPTED; 
            free(sequence);
        }
        // Advances all the frequencies of the symbols (they are unnecessary for this process)
        if (i < n_blocks - 1) {
            for (int k = 0; k <= n_simb; ++k) { // FIGURE OUT: Porque só funciona com <= em vez de <
                fscanf(f_freq, "%d[^;]", &block_size);// CORRIGIR: ignores return value
                fseek(f_freq, 1, SEEK_CUR);
            }
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

BTree add_tree(BTree decoder, char *code){
    BTree root=decoder;
    int i=0;
    //arvore tem que começar com um nodo
    while(decoder){
        if (code[i]=='1'){
            decoder=decoder->right;
        }
        else{
            decoder=decoder->left;
        }
        ++i;
    }
    for(;decoder[i];++i){
        decoder=malloc(sizeof(struct btree));
        decoder->left = decoder->right = NULL;
        if (code[i]=='1'){
            decoder=decoder->right;
        }
        else{
            decoder=decoder->left;
        }
    }

    return root;
}

_modules_error create_tree(char *path, int **blocks_sizes, int *size, BTree *decoder){
    FILE* f_cod = fopen(path,"rb");
    if(!f_cod) return _FILE_UNRECOGNIZABLE;
    // Reads header
    int j; 
    if (fscanf(f_freq, "@R@%d@", size) != 1) return _FILE_UNRECOGNIZABLE;
    *blocks_sizes = malloc (sizeof(int)*(*size));
    if (!(*blocks_sizes)) return _LACK_OF_MEMORY;
    // 
    for (int i = 0; i < size; ++i) {
        int block_size;
        if (fscanf(f_freq, "%d[^@]", &block_size) == 1)
            blocks_sizes[j++]=block_size;
        
        for (int k = 0; k <= n_simb; ++k) { // FIGURE OUT: Porque só funciona com <= em vez de <
            char * code;
            if(fscanf(f_freq, "%s[^;]", code)==1){
                *decoder = add_tree(*decoder,code);
            }
            fseek(f_freq, 1, SEEK_CUR);
        }
    }
}

