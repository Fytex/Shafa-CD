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
#define min(a,b) (a > b ? b : a) 
#define nsimb 256
#define RUN 32


uint32_t frequencies[256];

typedef struct Symbol 
{
    uint32_t freq;   // 2^32 bits it's enough to save 64MBytes
    uint8_t * code; // You need to use malloc and save the reference.
    uint8_t value; // is the same as the original array's position
} Symbol;

Symbol symbols[256];

// uint_8 * output[256];
void insertSort (Symbol * symbols, int left , int right)
{
    for (int i = left +1; i<= right ; i++ ){

        uint32_t tmpfreq = symbols[i].freq;
        Symbol freq = symbols[i];
        int j = i - 1;

        while (j >= left && symbols[j].freq < tmpfreq  ){
            symbols[j+1] = symbols[j];
            j--;
        }
        symbols[j+1] = freq;
    }
} 

void merge (Symbol * symbols, int l, int m, int r) 
{ 
      
    int len1 = m - l + 1, len2 = r - m; 
    Symbol left[len1], right[len2]; 
    for (int i = 0; i < len1; i++) 
        left[i] = symbols[l + i]; 
    for (int i = 0; i < len2; i++) 
        right[i] = symbols[m + 1 + i]; 
  
    int i = 0; 
    int j = 0; 
    int k = l; 
  
    while (i < len1 && j < len2){ 
        if (left[i].freq >= right[j].freq){ 
            symbols[k] = left[i]; 
            i++; 
        } 
        else{
            symbols[k] = right[j]; 
            j++; 
        } 
        k++; 
    } 
    for (;i<len1;i++)symbols[k++] = left[i];
    for (;j<len2;j++)symbols[k++] = right[j];
    
}  

void timSort(Symbol* symbols) 
{ 
    int n = nsimb;
    // Sort individual subarrays of size RUN.
    for (int i = 0; i < n; i+=RUN) 
        insertSort(symbols, i, min((i+31),(n-1))); 
  
    // Start merging from size RUN (or 32).  
    for (int size = RUN; size < n;size = 2*size) 
    {

        for (int left = 0; left < n;left += 2*size) 
        {// find ending point of left sub array and mid+1 is starting point of right sub array.
            int mid = left + size - 1; 
            int right = min((left + 2*size - 1),(n-1)); 
  
            // merge sub array left with sub array right.
            merge(symbols, left, mid, right); 
        } 
    } 
} 

uint32_t sumFreq (Symbol * symbols,int first , int last)
{
    uint32_t soma = 0;

    for (int i = first; i <= last ; i++)
        soma += symbols[i].frequency;

    return soma;
}


int bestDivision (Symbol * symbols, int first, int last)
{
    
    int division = first, total , mindif, dif;
    uint32_t g1 = 0 ;

    total = mindif = dif = sumFreq(symbols,first,last);

    while (dif == mindif){
        g1 = g1 + symbols[division].frequency;
        dif = abs(2*g1 -total);
            if (dif < mindif){
                division = division + 1 ;
                mindif = dif;
            }
            else
                dif = mindif +1 ;
    }

    return division;
        
}


void updateSymbol(uint32_t freq[])
{
    for (int i = 0; i < nsimb; ++i){               // loop to read all 256 symbols
        symbols[i].freq = frequencies[i];          // saves the frequency of the symbol number i in the struct
        // symbols[i].code = malloc(8*sizeof(uint32_t)); // allocates dynamic memory to the shannon-fano codes
        symbols[i].value = i;                      // saves his original position 
    }
}

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
    /* for (int j = 0; j < 256; j++)
        printf ("Elemento %d do array: %d\n", j, frequencies[j]);
    */
}

int readHeader(FILE *f)
{
    int numOfBlocks;
    char codType;
    fscanf(f, "@ %c @ %d @", &codType, &numOfBlocks);  // reads the code type, the number of blocks and points to the block size
/*  printf("Tipo de Codificação: %c\n", codType);
    printf("Número de Blocos: %d\n", numOfBlocks);
*/
    return numOfBlocks;                               // returns the number of blocks of the file

}

bool get_shafa_codes(const char * const path)
{
    return true;
}
