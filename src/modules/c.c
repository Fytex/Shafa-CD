#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "c.h"
#include "utils/extensions.h"

#define MAX_CODE_INT 32
#define NUM_SYMBOLS 256
#define NUM_OFFSETS 8


typedef struct {
    int index;
    int next; 
    uint8_t * code[MAX_CODE_INT + 1];
} CodesIndex;


int compress_to_file(FILE * const fd_file, FILE * const fd_shafa, const char * const codes_input, const int block_size)
{
    int n_chars;
    const char * codes_in_ptr = codes_input;
    CodesIndex * const table = calloc(NUM_OFFSETS * NUM_SYMBOLS, sizeof(CodesIndex));
    char tmp;
    int bit_idx, code_idx, byte;

    for (int syb_idx = 0; syb_idx < NUM_SYMBOLS; ++syb_idx) {
        for (code_idx = 0, byte = 0; *codes_in_ptr != ';' && code_idx < MAX_CODE_INT; ++code_idx) {
            for (bit_idx = 0; bit_idx < 8; ++bit_idx) {

                tmp = *codes_in_ptr++;
                if (tmp == '1') ++byte;
                else if (tmp == ';') {
                    if (bit_idx)
                        byte <<= 8 - bit_idx;
                    break;
                }
                else if (tmp != '0') {
                    free(table);
                    return _FILE_UNRECOGNIZABLE;
                }
            
                byte <<= 1;
            }
            table[syb_idx].code[code_idx] = byte;
            byte = 0;
        }

        table[syb_idx].next = (bit_idx != 8) ? bit_idx * NUM_SYMBOLS : 0;
        table[syb_idx].index = code_idx - 1;
    }

    /*
    i = 3
    next_byte_extra = (*codes & (2^i - 1)) << (8-i)
    *codes =>> i
    ++codes
    *codes = *codes | byte_extra
    */

   return 0;
}


_modules_error shafa_compress(char ** const path)
{
    FILE * fd_file, * fd_codes, * fd_shafa;
    char * path_file = *path;
    char * path_codes;
    char * path_shafa;
    char * block_input;
    char mode;
    int num_blocks, block_size;
    int error = _SUCCESS;

    
    
    // Create Codes's path string and Open Codes's handle

    path_codes = add_ext(path_file, CODES_EXT);

    if (path_codes) {
    
        fd_codes = fopen(path_codes, "rb");

        if (fd_codes) {

            if (fscanf(fd_codes, "@%c@%d", &mode, &num_blocks) == 2) {

                // Open File's handle
                fd_file = fopen(path_file, "rb");

                if (fd_file) {
    

                    // Create Shafa's path string and Open Shafa's handle

                    path_shafa = add_ext(path_file, SHAFA_EXT);

                    if (path_shafa) {
    
                        fd_shafa = fopen(path_shafa, "wb");

                        if (fd_shafa) {

                            block_input = malloc(33151); //sum 1 to 256 (worst case shannon fano) + 255 semicolons 


                            for (int i = 0; i < num_blocks; ++i) {

                                fscanf(fd_codes,"@%d@%[^@]s",&block_size,block_input);

                                // A função compress_to_file vai criar a tabela através da string dada e vai escrever no ficheiro
                                compress_to_file(fd_file, fd_codes, fd_shafa, block_size, block_input); // use semaphore (mutex) [only when multithreading]
                            }


                            fclose(fd_shafa);
                            free(path_file);
                            *path = path_shafa;
                        }
                        else {
                            free(path_shafa);
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

            fclose(fd_codes);
        }
        else
            error = _FILE_INACCESSIBLE;

        free(path_codes);
    }
    else
        error = _LACK_OF_MEMORY;

    return error;
}

int main (){

    shafa_compress("aaa.txt");
}