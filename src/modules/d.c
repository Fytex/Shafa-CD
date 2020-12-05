/*

Cabeçalho a preencher

*/
#include "d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



/*
Algum possível comentário
*/

bool rle_decompress(char* const path) 
{
    // Reading and loading file into the buffer
    FILE* f_rle = fopen(path, "rb");
    if (!f_rle) { // Memory allocation problem
        printf("Issue opening rle file\n");
        return false;
    }
    fseek(f_rle, 0, SEEK_END); // Goes to the end of the file
    int size_f = ftell(f_rle); // Saves the size of the file 
    rewind(f_rle); // Goes back to the beggining of the file
    char* buffer = malloc(sizeof(char)*size_f); // Allocates memory for all the characters in the file
    if (buffer == NULL) { // Memory problem
        printf("Issue loading rle file to buffer\n");
        return false;
    }
    int res = fread(buffer, 1, size_f, f_rle); // Loads the contents of the file into the buffer
    if (res != size_f) { // Conflict between the size read and the size that should've been read
        printf("Conflicting size after reading rle file\n");
        return false;
    }
   
    // Writing in new file
    FILE* f_origin = fopen("newfile.txt", "wb"); // Nota: mudar de acordo com terminações do ficheiro
    if (!f_origin) { // Memory allocation problem
        printf("Issue opening writing file\n");
        return false;
    } 
        
    int size_origin = 5000 ; // Nota: corrigir isto para uma função que leia do .FREQ o tamanho original descomprimido (reutilizada do modulo C)
    char* sequence = malloc(sizeof(char)*size_origin + 1); // Allocates memory to the decompressed content of the file
    if (!sequence) {// Memory allocation problem
        printf("Issue generating memory to string\n");
        return false;
    }
    int l = 0; // Index of string 
    for (int i = 0; i < size_f; ++i) { // Reads RLE file and decompresses it
        char simb = buffer[i];
        if (!simb) { // Detects format "00 symbol number_of_repetitions"
            simb = buffer[++i]; // Finds the symbol that repeats 
            int n_reps = buffer[++i]; // Saves the number of repetitions
            memset(sequence + l, simb, n_reps); // Places the symbol in the string along the string according the number of repetitions
            l += n_reps; // Advances the index to one that hasn't been filled
        }
        else { // Case where there aren't repetitions
            sequence[l] = simb; 
            ++l;
        }
    }
    sequence[l] = '\0'; 
    fwrite(sequence, 1, --l, f_origin); // Writes the string in the file
    free(sequence);
    return true;
}


bool shafa_decompress(char * const path)
{
    return true;
}