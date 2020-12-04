#include <stdio.h>
#include "utils.h"


void filesize (Files *file)
{
    
    if(file != NULL){
        
        long int length;
        FILE *fp;
        
        fp = fopen(file->filename,"rb+");
        
        if(fp){

            fseek(fp, 0L, SEEK_END);
		    file->filesize = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
        }

        fclose(fp);
   }
   
}

//Updates struct Files after block division. Default blocksize is 64K bytes
void blockdivision (Files *file){
    long int fsize;  

    fsize = file->filesize;

    if (fsize >= 1024){
        
        file->nblocks = fsize / file->blocksize; 

        int mod = fsize % file->nblocks;
    
        if (mod < 1024)         
            file->lastblocksize = file->blocksize + mod;

        else{
            file->nblocks++;
            file->lastblocksize = mod;
        }
    }
}
