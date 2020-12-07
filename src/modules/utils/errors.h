#ifndef UTILS_ERRORS_H
#define UTILS_ERRORS_H


typedef enum _error
{
    _SUCCESS = 0,
    _LACK_OF_MEMORY      = -1,
    _FILE_INACCESSIBLE   = -2,
    _FILE_UNRECOGNIZABLE = -3,
    _FILE_TOO_SMALL      = -4,
    _FILE_CORRUPTED      = -5
} _error;


/**
\brief Respective error's message
 @param num Error's Number
 @returns Error's message
*/
const char * errormsg(int num);


#endif //UTILS_ERRORS_H
