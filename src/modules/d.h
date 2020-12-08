#ifndef MODULE_D_H
#define MODULE_D_H

#include "utils/errors.h"

/**
\brief Decompresses file which was compressed with RLE's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @returns Error status
*/
<<<<<<< HEAD
_modules_error rle_decompress(char ** path);
=======
int rle_decompress(char ** path);
>>>>>>> b2016d84799ee94baaaee0461f60c315beb7ee03


/**
\brief Decompresses file which was compressed with Shannon Fano's algorithm and saves it to disk
 @param path Pointer to the original/RLE file's path
 @returns Error status
*/
<<<<<<< HEAD
_modules_error shafa_decompress(char ** path);
=======
int shafa_decompress(char ** path);
>>>>>>> b2016d84799ee94baaaee0461f60c315beb7ee03

#endif //MODULE_D_H