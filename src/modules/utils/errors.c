/************************************************
 *
 *  Author(s): Pedro Tavares
 *  Created Date: 5 Dec 2020
 *  Updated Date: 8 Dec 2020
 *
 ***********************************************/


#include "errors.h"

#define ERROR_MSG(_)                                                                                            \
    _(            _SUCCESS, "No error\n"                                                                  )     \
    _(     _LACK_OF_MEMORY, "Not enough memory for allocation\n"                                          )     \
    _(  _FILE_INACCESSIBLE, "File can't be accessed. Either lack of permissions or file doesn't exist\n"  )     \
    _(_FILE_UNRECOGNIZABLE, "File not recognized\n"                                                       )     \
    _( _FILE_STREAM_FAILED, "Can't communicate properly with file's stream\n"                             )     \
    _(     _FILE_TOO_SMALL, "File too small for decompression\n"                                          )
    

#define ERROR_CASE(NUM, MSG) case NUM: return MSG;

const char * error_msg(const int num)
{
    // Since this returns a const char * allocated at compile time typically placed in .rodata
    // So there won't be any Undefined Behaviour

    switch (num) {
        ERROR_MSG(ERROR_CASE)
    }

    return "Unknown error";
}
