/************************************************
 *
 *  Author(s): Francisco Neves, Leonardo Freitas
 *  Created Date: 3 Dec 2020
 *  Updated Date: 12 Dec 2020
 *
 ***********************************************/

#include "t.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define RUN 32
#define NUM_SYMBOLS 256
#define min(a,b)       \
({ typeof (a) _a = (a);\
   typeof (b) _b = (b);\
   _a < _b ? _a : _b; })

char codType;
int numOfBlocks;
uint8_t positions [NUM_SYMBOLS];
uint32_t frequencies[NUM_SYMBOLS];

/* typedef struct Symbol 
{
    uint32_t freq;   // 2^32 bits it's enough to save 64MBytes
    uint8_t * code; // You need to use malloc and save the reference.
    uint8_t value; // is the same as the original array's position
} Symbol;
*/
// Symbol symbols[256];

// uint_8 * output[256];

void insertSort (uint32_t frequencies[], uint8_t positions[], int left , int right)
{
    for (int i = left +1; i<= right ; i++ ){

        uint32_t tmpFreq = frequencies[i];
        uint8_t tmpPos = positions[i];
        uint32_t freq = frequencies[i];
        uint8_t pos = positions[i];
        int j = i - 1;

        while (j >= left && frequencies[j] < tmpFreq  ){
            frequencies[j+1] = frequencies[j];
            positions[j+1] = positions[j];
            j--;
        }
        frequencies[j+1] = freq;
        positions[j+1] = pos;
    }
} 

void merge (uint32_t frequencies[], uint8_t positions[], int l, int m, int r) 
{ 
      
    int len1 = m - l + 1, len2 = r - m; 
    uint32_t left[len1], right[len2]; 
    uint8_t posLeft[len1], posRight[len2];
    for (int i = 0; i < len1; i++){ 
        left[i] = frequencies[l + i]; 
        posLeft[i] = positions[l + i];
    }
    for (int i = 0; i < len2; i++){
        right[i] = frequencies[m + 1 + i]; 
        posRight[i] = positions[m + 1 + i];
    }
  
    int i = 0; 
    int j = 0; 
    int k = l; 
  
    while (i < len1 && j < len2){ 
        if (left[i] >= right[j]){ 
            frequencies[k] = left[i]; 
            positions[k] = posLeft[i];
            i++; 
        } 
        else{
            frequencies[k] = right[j]; 
            positions[k] = right[j];
            j++; 
        } 
        k++; 
    } 
    for (;i<len1;i++){
        frequencies[k++] = left[i];
        positions[k++] = posLeft[i];
    }
    for (;j<len2;j++){
        frequencies[k++] = right[j];
        positions[k++] = posRight[j];
    }
    
}  

void timSort(uint32_t frequencies[], uint8_t positions[]) 
{ 
    int n = NUM_SYMBOLS;
    // Sort individual subarrays of size RUN.
    for (int i = 0; i < n; i+=RUN) 
        insertSort(frequencies, positions, i, min((i+31),(n-1))); 
  
    // Start merging from size RUN (or 32).  
    for (int size = RUN; size < n;size = 2*size) 
    {

        for (int left = 0; left < n;left += 2*size) 
        {// find ending point of left sub array and mid+1 is starting point of right sub array.
            int mid = left + size - 1; 
            int right = min((left + 2*size - 1),(n-1)); 
  
            // merge sub array left with sub array right.
            merge(frequencies, positions, left, mid, right); 
        } 
    } 
} 

uint32_t sumFreq (uint32_t frequencies[] ,int first , int last)
{
    uint32_t soma = 0;

    for (int i = first; i <= last ; i++)
        soma += frequencies[i];

    return soma;
}


int bestDivision (uint32_t frequencies[], int first, int last)
{
    
    int division = first, total , mindif, dif;
    uint32_t g1 = 0 ;

    total = mindif = dif = sumFreq(frequencies,first,last);

    while (dif == mindif){
        g1 = g1 + frequencies[division];
        dif = abs(2*g1 -total);
            if (dif < mindif){
                division = division + 1 ;
                mindif = dif;
            }
            else
                dif = mindif +1 ;
    }

    return division - 1;
        
}

void startCodes (unsigned char *codes[NUM_SYMBOLS])
{
   for (int i = 0; i < NUM_SYMBOLS; ++i){
     codes[i] = malloc((NUM_SYMBOLS + 1) * sizeof(unsigned char));
     for (int j = 0; j < NUM_SYMBOLS + 1; ++j) codes[i][j] = '2';
   }
}

void startPositions(uint8_t positions[])
{
    for (int i = 0; i < NUM_SYMBOLS; ++i) positions[i] = i;
}

/* void updateSymbol(uint32_t freq[])
{
    for (int i = 0; i < nsimb; ++i){               // loop to read all 256 symbols
        symbols[i].freq = frequencies[i];          // saves the frequency of the symbol number i in the struct
        // symbols[i].code = malloc(8*sizeof(uint32_t)); // allocates dynamic memory to the shannon-fano codes
        symbols[i].value = i;                      // saves his original position 
    }
}
*/

int readBlock (FILE* f) 
{
    int blockSize;  
    fscanf(f, "%d", &blockSize);             // reads the current Block size
    fgetc(f);                               // gets @
    int frequency;                  
    for (int i = 0; i < NUM_SYMBOLS; i++) {       // loop to read all 256 symbols
        fscanf(f, "%d", &frequency);     // reads the frequency of the symbol number i
        frequencies[i] = frequency;     // updates the array with the frequency
        fgetc(f);                      // gets ;
    }
    /* for (int j = 0; j < 256; j++)
        printf ("Elemento %d do array: %d\n", j, frequencies[j]);
    */
}

void readHeader(FILE *f)
{
    fscanf(f, "@ %c @ %d @", &codType, &numOfBlocks);  // reads the code type, the number of blocks and points to the block size
/*  printf("Tipo de Codificação: %c\n", codType);
    printf("Número de Blocos: %d\n", numOfBlocks);
*/
}

bool get_shafa_codes(const char * const path)
{
    return true;
}
