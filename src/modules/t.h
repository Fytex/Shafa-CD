#ifndef MODULE_T_H
#define MODULE_T_H

#include "utils/errors.h"

/**
\brief Creates a table of Shanon Fano's codes and saves it to disk
 @param path Original/RLE file's path
 @returns Error status
*/
_modules_error get_shafa_codes(const char * path);

#endif //MODULE_T_H