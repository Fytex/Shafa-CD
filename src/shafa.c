#include <stdio.h>
#include <stdbool.h>

#include "modules/f.h"
#include "modules/t.h"
#include "modules/c.h"
#include "modules/d.h"

typedef struct Options {
    int block_size;
    char * file;
    char module;
    char algorithm;
} Options;


bool parse(const int argc, char * const argv[], Options * const options)
{
    char opt;
    char * key, * value;

    for (int i = 1; i < argc; i += 2) {
        key = argv[i];
        value = argv[i+1];

        if (key[0] != '-' || key[2] || value[1]) // Stop if exceeds limit
            return (-1);
        
        opt = *value;

        switch (key[1]) {
            case 'm': // Chooses module
                if (opt != 'f' && opt != 't' && opt != 'c' && opt != 'd')
                    return (-1);
                options->module = opt;
                break;
            case 'b': // K|m|M
                if (opt != 'K' && opt != 'm' && opt != 'M')
                    return (-1);
                options->block_size = opt;
                break;
            case 'c': // r -> rle force
                if (opt != 'r')
                    return (-1);
                options->algorithm = opt;
                break;
            case 'd': //  s|r
                if (opt != 's' && opt != 'r')
                    return (-1);
                options->algorithm = opt;
                break;
            default:
                return (-1);
        }
    }
    return options;
}


int main (const int argc, char * argv[])
{
    Options options = {0};

    if (!parse(argc, argv, &options))
        fprintf(stderr, "Error");

    return 0;
}
