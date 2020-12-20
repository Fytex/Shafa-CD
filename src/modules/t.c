/************************************************
 *
 *  Author(s): Francisco Neves, Leonardo Freitas
 *  Created Date: 3 Dec 2020
 *  Updated Date: 19 Dec 2020
 *
 ***********************************************/

#include "t.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utils/extensions.h"


#define RUN 32
#define NUM_SYMBOLS 256

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


uint8_t positions [NUM_SYMBOLS];
uint32_t frequencies[NUM_SYMBOLS];

void readBlock (char * codes_input) 
{
    char * ptr = codes_input;
    int auxfreq, nread=0;

    for (int i = 0; i < NUM_SYMBOLS; ++i){
        if ( sscanf(ptr, "%u[^;]",&frequencies[i]) == 1 ){
                     
            auxfreq = frequencies[i];
             
            while (auxfreq != 0){  
                auxfreq /= 10;  
                nread++;  
            }  
            if (nread == 0) ++nread;

            ptr += nread + 1; 
            nread = 0;
        }
        else           
            ++ptr;
    }
}

void insertSort (uint32_t frequencies[], uint8_t positions[], int left , int right)
{
    for (int i = left +1; i<= right ; i++){

        uint32_t tmpFreq = frequencies[i];
        uint8_t pos = positions[i];
        int j = i - 1;

        while (j >= left && frequencies[j] < tmpFreq){
            frequencies[j+1] = frequencies[j];
            positions[j+1] = positions[j];
            j--;
        }
        frequencies[j+1] = tmpFreq;
        positions[j+1] = pos;
    }
} 

void merge (uint32_t frequencies[], uint8_t positions[], int l, int m, int r) 
{ 
      
    int len1 = m - l + 1, len2 = r - m; 
    uint32_t freqLeft[len1], freqRight[len2]; 
    uint8_t posLeft[len1], posRight[len2];

    for (int i = 0; i < len1; i++){ 
        freqLeft[i] = frequencies[l + i]; 
        posLeft[i] = positions[l + i];
    }
    for (int i = 0; i < len2; i++){
        freqRight[i] = frequencies[m + 1 + i]; 
        posRight[i] = positions[m + 1 + i];
    }
  
    int i = 0, j = 0 , k = l;
     
    while (i < len1 && j < len2){ 
        if (freqLeft[i] >= freqRight[j]){ 
            frequencies[k] = freqLeft[i]; 
            positions[k] = posLeft[i];
            i++; 
        } 
        else{
            frequencies[k] = freqRight[j]; 
            positions[k] = posRight[j];
            j++; 
        } 
        k++; 
    } 
    for (;i<len1;i++){
        frequencies[k] = freqLeft[i];
        positions[k] = posLeft[i];
        k++;
    }
    for (;j<len2;j++){
        frequencies[k] = freqRight[j];
        positions[k] = posRight[j];
        k++;
    }
}  

void timSort(uint32_t frequencies[], uint8_t positions[]) 
{ 
    int n = NUM_SYMBOLS;
    // Sort individual subarrays of size RUN.
    for (int i = 0; i < n; i += RUN) 
        insertSort (frequencies, positions, i, min((i+31),(n-1))); 
  
    // Start merging from size RUN (or 32).  
    for (int size = RUN; size < n; size = 2*size) 
    {

        for (int left = 0; left < n; left += 2*size) 
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
    uint32_t g1 = 0;

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

void add_bit_to_code(char value, unsigned char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{   
    //printf ("\nValue Original: %c\n", value);
    //printf("End: %d     Start: %d\n", end, start);
    for(int i = start; i <= end; ++i){
        //printf("CODE RESULTANTE : %s\n", codes[i]);
       // printf ("Value no 1º for: %c\n", value);
        int flag = 1;
       for (int j = 0; j < NUM_SYMBOLS + 1 && flag; ++j)
          if  (codes[i][j] == '\0'){
         //     printf ("Value no 2º for: %c\n", value);
              codes[i][j] = value;
              flag = 0;
          }

    }
    
}

void sf_codes (uint32_t frequencies[], unsigned char codes[NUM_SYMBOLS][NUM_SYMBOLS], int start, int end)
{
    if (start != end){
       // printf("\nStart: %d\n", start);
       //printf("\nEnd: %d\n", end);
        
        int div = bestDivision(frequencies, start, end);
       // printf ("\nBestDivision = %d\n", div);
        add_bit_to_code('0', codes, start, div);
        add_bit_to_code('1', codes, div + 1, end);
        sf_codes(frequencies, codes, start, div);
        sf_codes(frequencies, codes, div + 1, end);
    }
}

int notNull (uint32_t frequencies[NUM_SYMBOLS])
{
    int r = 0;
    for (int i = NUM_SYMBOLS - 1; frequencies[i] == 0; --i) ++r;
    return (NUM_SYMBOLS - 1 - r);
}


/*
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

void updateSymbol(uint32_t freq[])
{
    for (int i = 0; i < nsimb; ++i){               // loop to read all 256 symbols
        symbols[i].freq = frequencies[i];          // saves the frequency of the symbol number i in the struct
        // symbols[i].code = malloc(8*sizeof(uint32_t)); // allocates dynamic memory to the shannon-fano codes
        symbols[i].value = i;                      // saves his original position 
    }
}


void readBlock (FILE* f) 
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
     for (int j = 0; j < 256; j++)
        printf ("Elemento %d do array: %d\n", j, frequencies[j]);
    
}

void readHeader(FILE *f)
{
    fscanf(f, "@ %c @ %d @", &codType, &numOfBlocks);  // reads the code type, the number of blocks and points to the block size
  printf("Tipo de Codificação: %c\n", codType);
    printf("Número de Blocos: %d\n", numOfBlocks);

}
*/

// This module should receive a .freq file, but shafa.c will handle that file and pass to this module the original file

_modules_error get_shafa_codes(const char * path)
{
    FILE * fd_file, * fd_freq, * fd_codes;
    char * path_freq;
    char * path_codes;
    char * block_input;
    char mode;
    int error = _SUCCESS;
    int num_blocks, block_size, freq_notnull;


    path_freq = add_ext (path, FREQ_EXT);

    if (path_freq){

        fd_freq = fopen (path_freq, "rb");

        if (fd_freq){

            if (fscanf(fd_freq, "@%c%d", &mode, &num_blocks) == 2){

                fd_file = fopen(path, "rb");

                if (fd_file){

                    path_codes = add_ext (path, CODES_EXT);

                    if (path_codes){

                        fd_codes = fopen (path_codes, "wb");

                        if (fd_codes){

                            for (int i = 0; i < num_blocks; ++i) {

                                unsigned char (* codes)[NUM_SYMBOLS] = calloc (1, sizeof(unsigned char [NUM_SYMBOLS][NUM_SYMBOLS]));

                                for (int j = 0; j < NUM_SYMBOLS; ++j)positions[j] = j;
                                for (int j = 0; j < NUM_SYMBOLS; ++j)frequencies[j] = 0;

                                fscanf(fd_freq, "@%d", &block_size);

                                block_input = malloc (9 * 256 + 255);
                                fscanf (fd_freq, "@%[^@]s", block_input);
                                //printf("String lida %s\n", block_input);

                                readBlock(block_input);
                                timSort(frequencies,positions);

                                freq_notnull = notNull(frequencies);

                                sf_codes(frequencies,codes,0,freq_notnull);

                                //Now it is just write to the .cod the codes
                                

                                free(codes);

                            }

                            fclose(fd_codes);
                                                       

                        }
                        else {
                            free (path_codes);
                            error = _FILE_INACCESSIBLE;
                        }
                   
                    }
                    else
                        error = _LACK_OF_MEMORY;

                    fclose(fd_file);
                    
                }
                else
                    error = _FILE_INACCESSIBLE;
            }
            else 
                error = _FILE_UNRECOGNIZABLE;
            
            fclose(fd_freq);
        }
        else 
            error = _FILE_INACCESSIBLE;
        
        free(path_freq);

    }
    else
        error = _LACK_OF_MEMORY;    

    return error;
}
