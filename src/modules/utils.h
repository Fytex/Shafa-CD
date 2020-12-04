//Struct Files that handles all information related to the files received
typedef struct Files {

    int filesize;
    int lastblocksize;
    int nblocks;
    int blocksize;
    char * filename;

} Files;


void filesize (Files *file);

void blockdivision (Files *files);