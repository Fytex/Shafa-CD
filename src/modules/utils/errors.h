#ifndef UTILS_ERRORS_H
#define UTILS_ERRORS_H


typedef enum _error // Use integers between 0-255 for safe integer cast to pointer in multithreading [Better not use negative numbers]
{                   // If this fills 256 integers then assert sizeof(void *) <= max(_modules_error) or change multithreading to pass errors as arguments only
    _SUCCESS                   = 0,
    _OUTSIDE_MODULE            = 1,
    _LACK_OF_MEMORY            = 2,
    _FILE_INACCESSIBLE         = 3,
    _FILE_UNRECOGNIZABLE       = 4,
    _FILE_STREAM_FAILED        = 5,
    _FILE_TOO_SMALL            = 6,
    _THREAD_CREATION_FAILED    = 7,
    _THREAD_TERMINATION_FAILED = 8,
} _modules_error;


/**
\brief Respective error's message
 @param num Error's Number
 @returns Error's message
*/
const char * error_msg(int num);


#endif //UTILS_ERRORS_H
