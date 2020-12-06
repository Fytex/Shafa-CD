/***************************************************
 *
 *  Author(s): Joao Carvalho, Ana Teixeira
 *  Created Date: 3 Dec 2020
 *  Updated Date: 6 Dec 2020
 *
 **************************************************/

#include "f.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool rle_compress(char * const path, const bool force_rle, const int block_size)
{
    FILE* f = fopen(path, "rb");
    if(!f) {
        //FILE_INACCESSIBLE 
        return false;
    }
    fseek(f, 0, SEEK_END);
    int size_f = ftell(f);
    rewind(f);
    if(size_f < 1024 && !force_rle) 
    {
        //FILE_TOOO_SMALL
        return false;
    }

    char* buffer = malloc(sizeof(char) * size_f);

    if(buffer == NULL) {
        //LACK OPF MEMORY
        return false;
    }
    int read = fread(buffer, 1, size_f, f);
    if ( read != size_f) {
        return false;
    }
    FILE* f_rle = fopen("new.rle", "rb");

    if(!f_rle) {
        
        return false;
    }

    char* block = malloc(sizeof(char) * block_size);

    if(!block) {
        //LACK OPF MEMORY
        return false;
    }
    int j, k;
    for(int i = 0, k=0; i < block_size && i < size_f; i = j, k++) {
        // ainda nao acabado
        int cons = 0;
        for(j = i; buffer[i] == buffer[j]; ++j, ++cons);
        if(cons >= 4) {
            // compression
        }
        else{
            j = i; 
            block[k] = buffer[i];
        } 

    }
    

    return true;
}


bool get_frequencies(char * const path, const int block_size)
{
    return true;
}