#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "modules/f.h"
#include "modules/t.h"
#include "modules/c.h"
#include "modules/d.h"
#include "modules/utils/extensions.h"

#define RLE_EXT ".rle"
#define CODES_EXT ".cod"
#define SHAFA_EXT ".shaf"

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})


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
    int i = 0;

    while (i < argc) {
        key = argv[i];

        if (key[0] != '-') {
            if (options->file) // There is a path to file already as an argument
                return 0;
            
            file = malloc(strlen(key) + strlen(RLE_EXT) + max(strlen(CODES_EXT), strlen(SHAFA_EXT)) + 1);
            if (!file)
                return 0;

            options->file = strcpy(file, key);
            ++i;
        }
        else {

            value = argv[i+1];

            if (key[2] || value[1]) // Stop if exceeds limit
                return 0;
        
            opt = *value;

            switch (key[1]) {
                case 'm': // Chooses module
                    switch (opt) {
                        case 'f':
                            options->module_f = true;
                            break;
                        case 't':
                            options->module_d = true;
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
                    if (opt != 'K' && opt != 'm' && opt != 'M')
                        return 0;
                    options->block_size = opt;
                    break;
                case 'c': // r -> rle force
                    if (opt != 'r')
                        return 0;
                    options->f_force_rle = true;
                    break;
                case 'd': //  s|r
                    if (opt == 's')
                        options->d_rle = true;
                    else if (opt == 'r')
                        options->d_shaf = true;
                    else
                        return 0;
                    break;
                default:
                    return 0;
            }
            i += 2;
        }
    }
    return 1;
}


int main (const int argc, char * argv[])
{
    Options options = {0};
    char * file;
    bool success;

    success = parse(argc, argv, &options);
    file = options.file;

    if (!file) {
        fprintf(stderr, "No file input");
        return 1;
    }
    else if (!success) {
        fprintf(stderr, "Wrong Options' syntax");
        free(options.file);
        return 1;
    }

    /*
                                            Execute modules

        It is the responsability of each module to append/remove extensions to the array `file`
                    which has enough memory to allocate all the extensions needed.
        Every module needs to check if the file that he needs exist and are accessible otherwise
                            they should return false in order to raise an error.
    */
    
    if (options.module_f) {
        success = rle_compress(file, options.f_force_rle); // Returns true if file was RLE compressed

        if (!success) {
            fprintf(stderr, "Module 'd': Something went wrong while compressing with RLE...");
            free(options.file);
            return 1;
        }
    
        success = get_frequencies(file);

        if (!success) {
            fprintf(stderr, "Module 'f': Something went wrong while creating frequencies' table...");
            free(options.file);
            return 1;
        }
    }

    if (options.module_t) {
        success = get_shafa_codes(file); // If file doesn't end in .rle then its considered an uncompressed one

        if (!success) {
            fprintf(stderr, "Module 't': Something went wrong...");
            free(options.file);
            return 1;
        }
    }

    if (options.module_c) {
        success = shafa_compress(file); // If file doesn't end in .rle then its considered an uncompressed one

        if (!success) {
            fprintf(stderr, "Module 'c': Something went wrong...");
            free(options.file);
            return 1;
        }
    }

    if (options.module_d) {

        if (options.d_shaf || !options.d_rle) { // Trigger: -m d -r s -r r || -m d -r s || -m d
            if (!check_extension(file, SHAFA_EXT)) {
                fprintf(stderr, "Module 'd': Wrong Extension... Should end in %s", SHAFA_EXT);
                free(options.file);
                return 1;
            }

            success = shafa_decompress(file);

            if (!success) {
                fprintf(stderr, "Module 'd': Something went wrong while decompressing with Shannon Fano...");
                free(options.file);
                return 1;
            }
        }
        
        if (options.d_rle || !options.d_shaf) { // Trigger: -m d -r s -r r || -m d -r r || -m d
            if (!check_extension(file, RLE_EXT)) {
                fprintf(stderr, "Module 'd': Wrong Extension... Should end in %s", RLE_EXT);
                free(options.file);
                return 1;
            }

            success = rle_decompress(file);

            if (!success) {
                fprintf(stderr, "Something went wrong in module 'd' while decompressing with RLE...");
                free(options.file);
                return 1;
            }
        }
    }
    free(options.file);
    return 0;
}
