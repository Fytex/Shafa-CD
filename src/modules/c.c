#include <stdint.h>
#include <stdio.h>
#include "c.h"


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

bool shafa_compress(char * const path)
{
    return true;
}