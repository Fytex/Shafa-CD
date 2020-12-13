#include <stdint.h>
#include <stdio.h>

#include "c.h"
#include "utils/extensions.h"

/*
void read_header (FILE * fp, FILE * ori)
{
    uint8_t max_code [256], table[256]={0};
    uint8_t * code, * symbols;
    int nblocks, i, j, blocksize;
    char mode[2]; 
    
    fp = fopen (fp,"rb");
    ori = fopen (ori,"rb");
   
 
    if (fscanf(fp,"@%c@%d@%d@",mode,&nblocks,&blocksize) == 3){
                
        symbols = (uint8_t*) malloc (blocksize);

        fread (symbols,1,blocksize,ori);
             
            for (j = 0; j <=255; j++){
            
                if (fscanf(fp,"%[^;]",max_code)!= 0){                  

                    for (i=0;i<strlen(max_code);i++)
                        if(max_code[i]=='@')break;

                fseek (fp,1,SEEK_CUR);      
                 
                code = (uint8_t*) malloc(strlen(max_code) / 8 );
                uint8_t byte = strtoul(max_code, NULL, 2);

                *code = byte;
            
                table[j] = *code;    
                }
                else{
                fseek(fp,1,SEEK_CUR);
                }                    
            } 
    }
}
*/

typedef struct CodesIndex {
    int index;
    int next; 
    code[257]; // 256 max code + 1 NULL
} CodesIndex;


_modules_error read_data(FILE * fd_codes, CodesIndex * codes_table_header)
{
    fscanf("@%s@") // just read the string and return the string
}



_modules_error shafa_compress(char ** path)
{
    FILE * fd_file, fd_codes, fd_shafa;
    char * path_file = *path;
    char * path_codes;
    char * path_shafa;
    int num_blocks, block_size;
    char mode;
    
    
    // Create Codes's path string and Open Codes's handle

    path_codes = add_ext(path_file, CODES_EXT);

    if (!path_codes)
        return _LACK_OF_MEMORY;
    
    fd_codes = fopen(path_codes, "rb");

    if (!fd_codes)
        return _FILE_INACCESSIBLE;

    if (fscanf(fd_codes, "@%c@%d@%d", mode, &num_blocks, &block_size) != 3)
        return _FILE_UNRECOGNIZABLE;


    // Open File's handle
    
    fd_file = fopen(path_file, "rb");

    if (!fd_file)
        return _FILE_INACCESSIBLE;
    

    // Create Shafa's path string and Open Shafa's handle

    path_shafa = add_ext(path_file, SHAFA_EXT);

    if (!path_shafa)
        return _LACK_OF_MEMORY;

    
    fd_shafa = fopen(path_shafa, "wb");

    if (!fd_shafa)
        return _FILE_INACCESSIBLE;

    
    for (int i = 0; i < num_blocks; ++i) {
        error = read_data(fd_codes, file_input); // se for apenas uma linha mais vale esquecer a função e por diretamente
        // A função compress_to_file vai criar a tabela através da string dada e vai escrever no ficheiro
        error = compress_to_file(fd_file, fd_shafa, codes_table_header, file_input); // use semaphore (mutex) [only when multithreading]
    }

    fclose(fd_codes);
    fclose(fd_file);
    fclose(fd_shafa);

    *path = path_shafa;

    free(path_codes);
    free(path_file);

    return _SUCCESS;
}