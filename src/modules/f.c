/***************************************************
 *
 *  Author(s): Ana Teixeira, João Carvalho
 *  Created Date: 3 Dec 2020
 *  Updated Date: 7 Dec 2020
 *
 **************************************************/

#include "f.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool rle_compress(char * const path, const bool force_rle, const int block_size)
{
    //Reading and loading file into the buffer
    FILE* f = fopen(path, "rb");
    if(!f) //_FILE_INACCESSIBLE
    {
        return false;
    }

    fseek(f, 0, SEEK_END); //Goes to the end of the file
    int size_f = ftell(f); //Saves the size of the file
    rewind(f); //Goes to the begining of the file

    if(size_f < 1024 && !force_rle) //_FILE_TOO_SMALL
    {
        return false;
    }

    char* buffer = malloc(sizeof(char) * size_f); //Allocates memory for all the characters in the file
    if(buffer == NULL) //_LACK_OF_MEMORY
    {
        return false;
    }

    int read = fread(buffer, 1, size_f, f); //Loads the content of the file into the buffer
    if (read != size_f) //something went wrong(saber qual erro se considera ser)
    {
        return false;
    }

    //Writing in a new file
    FILE* f_rle = fopen("new.rle", "rb");
    if(!f_rle) //_FILE_INACCESSIBLE (confirmar se se considera ser este o erro)
    {
        
        return false;
    }

    char* block = malloc(sizeof(char) * block_size); //Allocates memory for the compressed(or not) content of the file
    if(!block) //LACK_OF_MEMORY
    {
        return false;
    }

    //Looping variables(i,j,k,l), number of symbols int the beginning(i) and if compressed(comp_block_size)
    //and a flag to see if there was any compression(flag)
    int i, j, k, l, comp_block_size = 0, flag = 0;
    //Cycle that goes through the first block of symbols of the file
    for(i = 0, k=0; i < block_size && i < size_f; i = j, k++) {
        int n_reps = 0; //Number of repetitions of a symbol
        for(j = i; buffer[i] == buffer[j]; ++j, ++n_reps); //Counts the number of repetitions of a symbol
        if(n_reps >= 4 || !buffer[i]) //If a symbol repits itself 4 times or more or if the symbol is null
        {
            flag = 1; //The flag indicates now that something got compressed
            //Cycle that puts the compressed symbols in the 'block' array(não está acabada - precisa de correção)
            for(l = k; n_reps; l++, --n_reps)
            {
                block[k] = NULL;
                block[k+1] = buffer[i];//fazer função para calcular ascii de um simbolo
                block[k+2] = 'n_reps';
            }
        }
        else //If there were no repited symbols(nothing got compressed)
        {
            j = ++i; 
            block[k] = buffer[i];
        } 
    }
    if(flag) //If something got compressed
        for(; block[comp_block_size]; comp_block_size++); //Calculates the number of symbols after compression
    double compression_ratio = (i - comp_block_size)/i; //Calculates the compression ratio

    return true;
}


bool get_frequencies(char * const path, const int block_size)
{
    return true;
}