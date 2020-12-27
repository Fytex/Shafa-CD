/************************************************
 *
 *  Author(s): Pedro Tavares
 *  Created Date: 3 Dec 2020
 *  Updated Date: 20 Dec 2020
 *
 ***********************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "modules/f.h"
#include "modules/t.h"
#include "modules/c.h"
#include "modules/d.h"
#include "modules/utils/file.h"
#include "modules/utils/errors.h"
#include "modules/utils/extensions.h"


typedef struct {
    int block_size;
    bool module_f;
    bool module_t;
    bool module_c;
    bool module_d;
    bool f_force_rle;
    bool f_force_freq;
    bool d_shaf;
    bool d_rle;
} Options;


static bool parse(const int argc, char * const argv[], Options * const options, char ** const file)
{
    char opt;
    char * key, * value;

    for (int i = 1; i < argc; ++i) { // argv[0] == "./shafa"
        key = argv[i];
        
        if (key[0] != '-') {
            if (*file) // There is a path to file already as an argument
                return false;

            *file = key;
        }
        else {

            if (++i >= argc)
                return false;

            value = argv[i];

            if (strlen(key) != 2 || strlen(value) != 1)
                return false;
        
            opt = *value;

            switch (key[1]) {
                case 'm': // Chooses module
                    switch (opt) {
                        case 'f':
                            options->module_f = true;
                            break;
                        case 't':
                            options->module_t = true;
                            break;
                        case 'c':
                            options->module_c = true;
                            break;
                        case 'd':
                            options->module_d = true;
                            break;
                        default:
                            return 0;
                    }
                    break;
                case 'b': // K|m|M
                    switch (opt) {
                        case 'K':
                            options->block_size = _640KiB;
                            break;
                        case 'm':
                            options->block_size = _8MiB;
                            break;
                        case 'M':
                            options->block_size = _64MiB;
                            break;
                        default:
                            return 0;
                    }
                    break;
                case 'c': // r -> rle force    |    f -> freq force
                    if (opt == 'r')
                        options->f_force_rle = true;
                    else if (opt == 'f')
                        options->f_force_freq = true;
                    else
                        return false;
                    break;
                case 'd': // s|r
                    if (opt == 's')
                        options->d_shaf = true;
                    else if (opt == 'r')
                        options->d_rle = true;
                    else
                        return false;
                    break;
                default:
                    return false;
            }
        }
    }
    return true;
}


/*
                                            Execute modules

         It is the responsability of each module to append/remove extensions to the filename and
                       assign the new filename's reference to the argument `path`.
        Every module needs to check if the files that they need exist and are accessible otherwise
           they should return an int enumerated in _modules_error from modules/utils/errors.h
*/
_modules_error execute_modules(Options options, char ** const ptr_file) // better copying only a few bytes instead of dereferncing all of them
{
    _modules_error error;
    char * tmp_file;
    
    if (options.module_f) {
        error = freq_rle_compress(ptr_file, options.f_force_rle, options.f_force_freq, options.block_size); // Returns true if file was RLE compressed

        if (error) {
            fputs("Module d: Something went wrong while compressing with RLE or creating frequencies' table...\n", stderr);
            return error;
        }
    }

    if (options.module_t) {

        if (!options.module_f) {
            if (check_ext(*ptr_file, FREQ_EXT)) {
                tmp_file = rm_ext(*ptr_file);

                if (!tmp_file)
                    return _LACK_OF_MEMORY;

                free(*ptr_file);
                *ptr_file = tmp_file;
            }       
            else {
                fprintf(stderr, "Module d: Wrong extension... Should end in %s\n", FREQ_EXT);
                return _OUTSIDE_MODULE;
            }
        }

        error = get_shafa_codes(*ptr_file); // If file doesn't end in .rle then its considered an uncompressed one

        if (error) {
            fputs("Module t: Something went wrong...\n", stderr);
            return error;
        }
    }

    if (options.module_c) {

        if (options.module_f && !options.module_t) { // Conflict
            fputs("Module c: Can't execute module 'c' after 'f' without 't'...\n", stderr);
            return _OUTSIDE_MODULE;
        }

        error = shafa_compress(ptr_file); // If file doesn't end in .rle then its considered an uncompressed one

        if (error) {
            fputs("Module c: Something went wrong...\n", stderr);
            return error;
        }
    }

    if (options.module_d) {

        if ((options.module_f && (!options.module_t || !options.module_c)) || (options.module_t && !options.module_c)) { // Conflict
            fputs("Module d: Can't execute module 'd' after 'f' without 't' or 'c', nor execute it after 't'  without 'c'...\n", stderr);
            return _OUTSIDE_MODULE;
        }

        if (options.d_shaf || !options.d_rle) { // Trigger: NULL | -m d | -m d -d s

            if (!check_ext(*ptr_file, SHAFA_EXT)) { 
                if (options.d_shaf) { // User forced execution of Shannon Fano's decompression
                    fprintf(stderr, "Module d: Wrong extension... Should end in %s\n", SHAFA_EXT);
                    return _OUTSIDE_MODULE;
                }
            }
            else {

                error = shafa_decompress(ptr_file, options.d_rle || !options.d_shaf); // RLE => Trigger: NULL | -m d

                if (error) {
                    fputs("Module d: Something went wrong while decompressing...\n", stderr);
                    return error;
                }
            }
        }
        
        if (options.d_rle && !options.d_shaf) { // Trigger: -m d -d r

            if (!check_ext(*ptr_file, RLE_EXT)) { 
                fprintf(stderr, "Module d: Wrong extension... Should end in %s\n", RLE_EXT);
                return _OUTSIDE_MODULE;
            }

            error = rle_decompress(ptr_file, &blocks_size);

            if (error) {
                fputs("Module d: Something went wrong while decompressing...\n", stderr);
                return error;
            }
        }
    }
    return _SUCCESS;
}


int main (const int argc, char * const argv[])
{
    Options options = {0}; // Reference C99 Standard 6.7.8.21
    char * file = NULL;
    int error;

    if (argc <= 1) {
        fputs("No file input\n", stderr);
        return 1;
    }

    if (!parse(argc, argv, &options, &file)) {
        fputs("Wrong Options' syntax\n", stderr);
        return 1;
    }

    if (!file) {
        fputs("No file input\n", stderr);
        return 1;
    }

    // Have to otherwise it will raise error if some modules tries to free it in order to change the pointer to the new file's path
    file = add_ext(file, ""); // does the same as `strdup` from <string.h> which is not supported in c17

    if (!file) {
        fputs("Not enough memory\n", stderr);
        return 1;
    }



    if (!options.module_f && !options.module_t && !options.module_c && !options.module_d)
        options.module_f = options.module_t = options.module_c = options.module_d = 1;

    // Can't do the same for `options.d_shaf` and `options.d_rle` since we would lost information
    // about the user forcing Shannon Fano's decompression in case they passed a `.rle` file
    // which should raise a custom error
    
    if (!options.block_size)
        options.block_size = _64KiB;
        
    error = execute_modules(options, &file);
    free(file);

    if (error) {
        if (error != _OUTSIDE_MODULE)
            fputs(error_msg(error), stderr);
        return 1;
    }

    return 0;
}
