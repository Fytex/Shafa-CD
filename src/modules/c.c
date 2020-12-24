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


static uint8_t * binary_coding(CodesIndex (* const table)[NUM_SYMBOLS], const uint8_t * restrict block_input, const unsigned long block_size, unsigned long * const new_block_size)
{
    uint8_t * output;
    CodesIndex * symbol;
    int offset = 0, num_bytes_code;
    uint8_t * code;

    uint8_t * const block_output = calloc(block_size, sizeof(uint8_t));

    if (!block_output)
        return NULL;

    output = block_output;
    
    for (unsigned long idx = 0; idx < block_size; ++idx) {
        symbol = &table[offset][*block_input++];

        num_bytes_code = symbol->index;
        code = symbol->code;

        for (int i = 0; i < num_bytes_code; ++i)
            *output++ |= *code++;
        
        *output |= *code;

        offset = symbol->next;
    }

    *new_block_size = output - block_output + 1;

    return block_output;
}


static int compress_to_file(FILE * const fd_file, FILE * const fd_shafa, const char * block_codes, const uint8_t * const block_input, const unsigned long block_size, uint8_t ** const block_output, unsigned long * const new_block_size)
{
    CodesIndex header_symbol_row, *symbol_row;
    char cur_char, next_char;
    int bit_idx, code_idx;
    uint8_t byte, next_byte_prefix = 0, mask;

    CodesIndex (* table)[NUM_SYMBOLS] = calloc(1, sizeof(CodesIndex[NUM_OFFSETS][NUM_SYMBOLS]));

    if (!table)
        return _LACK_OF_MEMORY;

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
            table[0][syb_idx].code[code_idx] = byte;
        }

        cur_char = next_char;
        next_char = *block_codes++;

        if (code_idx > 0) {
            table[0][syb_idx].next = (bit_idx != 8) ? (bit_idx + 1) : 0;
            table[0][syb_idx].index = code_idx - (bit_idx < 8 ? 1 : 0);
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

        header_symbol_row = table[0][idx];

        for (int offset = 1, bit_offset; offset < NUM_OFFSETS; ++offset) {

            symbol_row = &table[offset][idx];

            bit_offset = header_symbol_row.next + offset;
            new_index = header_symbol_row.index + (bit_offset < 8 ? 0: 1);
            symbol_row->index = new_index;
            symbol_row->next = (bit_offset < 8) ? bit_offset : bit_offset - 8;

            next_byte_prefix = 0;
            for (int code_idx = 0; code_idx < new_index; ++code_idx) {

                byte = header_symbol_row.code[code_idx];
                symbol_row->code[code_idx] = (byte >> offset) | (next_byte_prefix << (8 - offset));

                mask = (1 << offset) - 1;
                next_byte_prefix = byte & mask;

            }

            // This will only execute if next != 0 (which is the same as bit_offset != 8) Because this iteration is unnecessary
            if (bit_offset != 8) {
                byte = header_symbol_row.code[new_index];
                symbol_row->code[new_index] = (byte >> offset) | (next_byte_prefix << (8 - offset));
            }
        }
    }

    
    /*
    /
    /    Write compressed code
    /
    */

    
    *block_output = binary_coding(table, block_input, block_size, new_block_size);

    if (!*block_output)
        return _LACK_OF_MEMORY;

    /* Print output to stdio

    printf("%lu\n", *new_block_size);
    for (unsigned long i = 0; i < *new_block_size; i++)
        printf("%02X", *block_output[i]);
    puts("");
    
    */

    return _SUCCESS;
}


_modules_error shafa_compress(char ** const path)
{
    FILE * fd_file, * fd_codes, * fd_shafa;
    char * path_file = *path;
    char * path_codes;
    char * path_shafa;
    char * block_codes;
    char mode;
    long long num_blocks;
    unsigned long block_size, new_block_size;
    int error = _SUCCESS;
    uint8_t * block_input, * block_output = NULL;

    
    
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

                                    for (long long i = 0; i < num_blocks && !error; ++i) {

                                        if (fscanf(fd_codes,"@%lu@%33151[^@]", &block_size, block_codes) == 2) {
                                            block_input = malloc(block_size);

                                            if (block_input) {
                                                if (fread(block_input, sizeof(uint8_t), block_size, fd_file) == block_size) {
                                                    error = compress_to_file(fd_file, fd_shafa, block_codes, block_input, block_size, &block_output, &new_block_size); // use semaphore (mutex) [only when multithreading]
                                                    
                                                    if (!error) {
                                                        if (fprintf(fd_shafa, "@%lu", new_block_size) >= 2) {

                                                            if (fwrite(block_output, sizeof(uint8_t), new_block_size, fd_shafa) != new_block_size)
                                                                error = _FILE_STREAM_FAILED;

                                                        }

                                                        free(block_output);
                                                    }
                                                    else
                                                        error = _LACK_OF_MEMORY;
                                                }
                                                else
                                                    error = _FILE_STREAM_FAILED;
                                                
                                                free(block_input);
                                            }
                                            else
                                                error = _LACK_OF_MEMORY;
                                        }
                                        else
                                            error = _FILE_STREAM_FAILED;
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