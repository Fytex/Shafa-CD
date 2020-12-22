#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "c.h"
#include "utils/extensions.h"

#define MAX_CODE_INT 32
#define NUM_SYMBOLS 256
#define NUM_OFFSETS 8


typedef struct {
    int index;
    int next; 
    uint8_t code[MAX_CODE_INT + 1];
} CodesIndex;


static int compress_to_file(FILE * const fd_file, FILE * const fd_shafa, const char * block_codes, const uint8_t * const block_input, const int block_size)
{
    CodesIndex * table = calloc(NUM_OFFSETS * NUM_SYMBOLS, CodesIndex));
    CodesIndex header_symbol_row, *symbol_row;
    char cur_char, next_char;
    int bit_idx, code_idx;
    uint8_t byte, next_byte_prefix = 0, mask;


    /*
    /
    /    Table's header initialization
    /
    */

    cur_char = *block_codes++;
    next_char = *block_codes++;
    for (int syb_idx = 0; syb_idx < NUM_SYMBOLS; ++syb_idx) {

        code_idx = 0;

        for ( ; cur_char != ';' && cur_char != '\0' && code_idx < MAX_CODE_INT; ++code_idx) {
            byte = 0;
            for (bit_idx = 0; bit_idx < 8; ++bit_idx) {

                if (cur_char == '1')
                    ++byte;
                else if (cur_char != '0') {
                    free(table);
                    return _FILE_UNRECOGNIZABLE;
                }

                if (bit_idx < 7) {
                    if (next_char == ';') {
                        byte <<= 7 - bit_idx;
                        cur_char = next_char;
                        next_char = *block_codes++;
                        break;
                    }
                    else if (next_char == '\0') {
                        byte <<= 7 - bit_idx;
                        cur_char = next_char;
                        break;
                    }
                    else
                        byte <<= 1;
                }

                cur_char = next_char;
                next_char = *block_codes++;

            }
            table[syb_idx].code[code_idx] = byte;
        }

        cur_char = next_char;
        next_char = *block_codes++;

        if (code_idx > 0) {
            table[syb_idx].next = (bit_idx != 8) ? (bit_idx + 1) * NUM_SYMBOLS : 0;
            table[syb_idx].index = code_idx - (bit_idx < 8 ? 1 : 0);
        }

    }
   

    if (next_char != '\0') { // Check whether file is actually correct (Not required but it is an assert)
        free(table);
        return _FILE_UNRECOGNIZABLE;
    }


    /*
    /
    /    Table's body initialization
    /
    */

    for (int idx = 0, new_index; idx < NUM_SYMBOLS; ++idx) {

        header_symbol_row = table[idx];

        for (int offset = 1, bit_offset; offset < NUM_OFFSETS; ++offset) {

            symbol_row = &table[offset * NUM_SYMBOLS + idx];

            bit_offset = header_symbol_row.next / NUM_SYMBOLS + offset;
            new_index = header_symbol_row.index + (bit_offset < 8 ? 0: 1);
            symbol_row->index = new_index;
            symbol_row->next = ((bit_offset < 8) ? bit_offset : bit_offset - 8) * NUM_SYMBOLS;

            next_byte_prefix = 0;
            for (int code_idx = 0; code_idx < new_index; ++code_idx) {

                byte = header_symbol_row.code[code_idx];
                symbol_row->code[code_idx] = (byte >> offset) | (next_byte_prefix << (8 - offset));

                mask = (1 << offset) - 1;
                next_byte_prefix = byte & mask;

            }

            // This will only execute if next != 8*N. where N = 256 symbols. Because this iteration is unnecessary
            if (bit_offset != 8) {
                byte = header_symbol_row.code[new_index];
                symbol_row->code[new_index] = (byte >> offset) | (next_byte_prefix << (8 - offset));
            }
        }
    }

    
    /*
    /
    /    Write to .shaf
    /
    */

   for (int idx = 0; idx < block_size; ++idx) {

        byte = *block_input;
        symbol_row = table + byte;

    
    


   }

}


_modules_error shafa_compress(char ** const path)
{
    FILE * fd_file, * fd_codes, * fd_shafa;
    char * path_file = *path;
    char * path_codes;
    char * path_shafa;
    char * block_codes, * block_input;
    char mode;
    long long num_blocks;
    int block_size;
    int error = _SUCCESS;

    
    
    // Create Codes's path string and Open Codes's handle

    path_codes = add_ext(path_file, CODES_EXT);

    if (path_codes) {
    
        fd_codes = fopen(path_codes, "rb");

        if (fd_codes) {

            if (fscanf(fd_codes, "@%c@%lld", &mode, &num_blocks) == 2) {

                // Open File's handle
                fd_file = fopen(path_file, "rb");

                if (fd_file) {
    

                    // Create Shafa's path string and Open Shafa's handle

                    path_shafa = add_ext(path_file, SHAFA_EXT);

                    if (path_shafa) {
    
                        fd_shafa = fopen(path_shafa, "wb");

                        if (fd_shafa) {

                            block_codes = malloc(33153); //sum 1 to 256 (worst case shannon fano) + 255 semicolons + 1 byte NULL + 1 extra byte for algorithm efficiency avoiding 256 compares

                           if (block_codes) {

                                if (fprintf(fd_shafa, "@%lld", num_blocks) >= 2) {

                                    for (long long i = 0; i < num_blocks; ++i) {

                                        if (fscanf(fd_codes,"@%d@%33151[^@]s", &block_size, block_codes) != 2) {
                                            error = _FILE_STREAM_FAILED;
                                            break;
                                        }

                                        block_input = malloc(block_size);

                                        if (!block_input) {
                                            error = _LACK_OF_MEMORY;
                                            break;
                                        }
                                        
                                        if (fread(block_input, sizeof(uint8_t), block_size, fd_file) != block_size) {
                                            error = _FILE_STREAM_FAILED;
                                            break;
                                        }

                                        // A função compress_to_file vai criar a tabela através da string dada e vai escrever no ficheiro
                                        compress_to_file(fd_file, fd_shafa, block_codes, block_input, block_size); // use semaphore (mutex) [only when multithreading]

                                    }
                                }
                                else
                                    error = _FILE_STREAM_FAILED;
                            
                            free(block_codes);
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