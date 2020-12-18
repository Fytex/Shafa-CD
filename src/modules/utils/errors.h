#ifndef UTILS_ERRORS_H
#define UTILS_ERRORS_H


typedef enum _error
{
    _SUCCESS = 0,
    _OUTSIDE_MODULE      =  1,
    _LACK_OF_MEMORY      = -1,
    _FILE_INACCESSIBLE   = -2,
    _FILE_UNRECOGNIZABLE = -3,
    _FILE_STREAM_FAILED  = -4,
    _FILE_TOO_SMALL      = -5,
} _modules_error;


/**
\brief Respective error's message
 @param num Error's Number
 @returns Error's message
*/
const char * error_msg(int num);


#endif //UTILS_ERRORS_H
