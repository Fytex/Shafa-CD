#ifndef UTILS_ERRORS_H
#define UTILS_ERRORS_H


typedef enum _error // Use integers between 0-255 for safe integer cast to pointer in multithreading
{                   // If it once became wid
    _SUCCESS             = 0,
    _OUTSIDE_MODULE      = 1,
    _LACK_OF_MEMORY      = 2,
    _FILE_INACCESSIBLE   = 3,
    _FILE_UNRECOGNIZABLE = 4,
    _FILE_STREAM_FAILED  = 5,
    _FILE_TOO_SMALL      = 6,
} _modules_error;


/**
\brief Respective error's message
 @param num Error's Number
 @returns Error's message
*/
const char * error_msg(int num);


#endif //UTILS_ERRORS_H
