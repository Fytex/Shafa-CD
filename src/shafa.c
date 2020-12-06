/************************************************
 *
 *  Author(s): Pedro Tavares
 *  Created Date: 3 Dec 2020
 *  Updated Date: 5 Dec 2020
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
#include "modules/utils/extensions.h"

enum {
    _64KiB  = 65536,
    _640KiB = 655360,
    _8MiB   = 8388608,
    _64MiB  = 67108864
};


typedef struct Options {
    int block_size;
    char * file;
    bool module_f;
    bool module_t;
    bool module_c;
    bool module_d;
    bool f_force_rle;
    bool d_shaf;
    bool d_rle;
} Options;


bool parse(const int argc, char * const argv[], Options * const options)
{
    char opt;
    char * file;
    char * key, * value;

    for (int i = 1; i < argc; ++i) { // argv[0] == "./shafa"
        key = argv[i];
        
        if (key[0] != '-') {
            if (options->file) // There is a path to file already as an argument
                return 0;

            file = malloc(strlen(key) + strlen(RLE_EXT) + strlen(SHAFA_EXT) + 1);
            if (!file)
                return 1; // Fake success. Main will trigger No Input File Found as an error

            options->file = strcpy(file, key);
        }
        else {

            if (++i >= argc)
                return 0;

            value = argv[i];

            if (strlen(key) != 2 || strlen(value) != 1)
                return 0;
        
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
                case 'c': // r -> rle force
                    if (opt != 'r')
                        return 0;
                    options->f_force_rle = true;
                    break;
                case 'd': // s|r
                    if (opt == 's')
                        options->d_shaf = true;
                    else if (opt == 'r')
                        options->d_rle = true;
                    else
                        return 0;
                    break;
                default:
                    return 0;
            }
        }
    }
    return 1;
}


int main (const int argc, char * const argv[])
{
    Options options = {0};
    char * file;
    bool success;

    if (argc <= 1) {
        fprintf(stderr, "No file input\n");
        return 1;
    }

    success = parse(argc, argv, &options);
    file = options.file;

    if (!success) {
        fprintf(stderr, "Wrong Options' syntax\n");

        if (!file)
            free(options.file);

        return 1;
    }
    else if (!file) { // !file must be after !success since parser can leave before reading file's path if insuccess
        fprintf(stderr, "No file input (Rare Case: No memory left)\n");
        return 1;
    }

    if (!options.module_f && !options.module_t && !options.module_c && !options.module_d)
        options.module_f = options.module_t = options.module_c = options.module_d = 1;

    // Can't do the same for `options.d_shaf` and `options.d_rle` since we would lost information
    // about the user forcing Shannon Fano's decompression in case they passed a `.rle` file
    // which should raise a custom error
    
    if (!options.block_size)
        options.block_size = _64KiB;
        

    /*
                                            Execute modules

        It is the responsability of each module to append/remove extensions to the array `file`
                    which has enough memory to allocate all the extensions needed.
        Every module needs to check if the file that he needs exist and are accessible otherwise
                            they should return false in order to raise an error.
    */
    
    if (options.module_f) {
        success = rle_compress(file, options.f_force_rle, options.block_size); // Returns true if file was RLE compressed

        if (!success) {
            fprintf(stderr, "Module 'd': Something went wrong while compressing with RLE...\n");
            free(options.file);
            return 1;
        }
    
        success = get_frequencies(file, options.block_size);

        if (!success) {
            fprintf(stderr, "Module 'f': Something went wrong while creating frequencies' table...\n");
            free(options.file);
            return 1;
        }
    }

    if (options.module_t) {
        success = get_shafa_codes(file); // If file doesn't end in .rle then its considered an uncompressed one

        if (!success) {
            fprintf(stderr, "Module 't': Something went wrong...\n");
            free(options.file);
            return 1;
        }
    }

    if (options.module_c) {

        if (options.module_f && !options.module_t) { // This is the only case where we can't check for extensions and has conflict
            fprintf(stderr, "Module 'c': Can't execute module 'c' after 'f' without 't'...\n");
            free(options.file);
            return 1;
        }

        success = shafa_compress(file); // If file doesn't end in .rle then its considered an uncompressed one

        if (!success) {
            fprintf(stderr, "Module 'c': Something went wrong...\n");
            free(options.file);
            return 1;
        }
    }

    if (options.module_d) {

        if (options.d_shaf || !options.d_rle) { // Trigger: NULL | -m d | -m d -d s

            if (!check_extension(file, SHAFA_EXT)) { 
                if (options.d_shaf) { // User forced execution of Shannon Fano's decompression
                    fprintf(stderr, "Module 'd': Didn't go through all past modules or wrong extension... Should end in %s\n", SHAFA_EXT);
                    free(options.file);
                    return 1;
                }
            }
            else {

                success = shafa_decompress(file);

                if (!success) {
                    fprintf(stderr, "Module 'd': Something went wrong while decompressing with Shannon Fano...\n");
                    free(options.file);
                    return 1;
                }
            }
        }
        
        if (options.d_rle || !options.d_shaf) { // Trigger: NULL | -m d | -m d -d r
            if (!check_extension(file, RLE_EXT)) {
                fprintf(stderr, "Module 'd': Didn't go through all past modules or wrong extension... Should end in %s\n", RLE_EXT);
                free(options.file);
                return 1;
            }

            success = rle_decompress(file);

            if (!success) {
                fprintf(stderr, "Something went wrong in module 'd' while decompressing with RLE...\n");
                free(options.file);
                return 1;
            }
        }
    }
    free(options.file);
    return 0;
}
