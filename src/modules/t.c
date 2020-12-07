/************************************************
 *
 *  Author(s): Francisco Neves, Leonardo Freitas
 *  Created Date: 3 Dec 2020
 *  Updated Date: 7 Dec 2020
 *
 ***********************************************/


#include "t.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t frequencies[256];

typedef struct Symbol 
{
    uint32_t freq;   // 2^32 bits it's enough to save 64MBytes
    uint8_t * code; // You need to use malloc and save the reference.
    uint8_t value; // is the same as the original array's position
} Symbol;

Symbol symbols[256];

// uint_8 * output[256];

void readBlock (FILE* f) {
    int blockSize;  
    fscanf(f, "%d", &blockSize);             // reads the current Block size
    fgetc(f);                               // gets @
    int frequency;                  
    for (int i = 0; i < 256; i++) {       // loop to read all 256 symbols
        fscanf(f, "%d", &frequency);     // reads the frequency of the symbol number i
        frequencies[i] = frequency;     // updates the array with the frequency
        fgetc(f);                      // gets ;
    }
    for (int j = 0; j < 256; j++)
        printf ("Elemento %d do array: %d\n", j, frequencies[j]);
    
}

int readHeader(FILE *f)
{
    int numOfBlocks;
    char codType;
    fscanf(f, "@ %c @ %d @", &codType, &numOfBlocks);  // reads the code type, the number of blocks and points to the block size
    printf("Tipo de Codificação: %c\n", codType);
    printf("Número de Blocos: %d\n", numOfBlocks);

    return numOfBlocks;                               // returns the number of blocks of the file

}

bool get_shafa_codes(const char * const path)
{
    return true;
}