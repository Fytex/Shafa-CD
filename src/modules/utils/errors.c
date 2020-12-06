/************************************************
 *
 *  Author(s): Pedro Tavares
 *  Created Date: 5 Dec 2020
 *  Updated Date: 6 Dec 2020
 *
 ***********************************************/


#include "errors.h"

#define ERROR_MSG(_)                                                                                          \
    _(            _SUCCESS, "No error"                                                                  )     \
    _(     _LACK_OF_MEMORY, "Not enough memory for allocation"                                          )     \
    _(  _FILE_INACCESSIBLE, "File can't be accessed. Either lack of permissions or file doesn't exist"  )     \
    _(_FILE_UNRECOGNIZABLE, "File not recognized"                                                       )     \
    _(     _FILE_TOO_SMALL, "File too small for decompression"                                          )     \
    _(     _FILE_CORRUPTED, "File is corrupted"                                                         )     \

#define ERROR_CASE(NUM, MSG) case NUM: return MSG;

const char * errormsg(const int num)
{
    // Since this returns a const char * allocated at compile time typically placed in .rodata
    // So there won't be any Undefined Behaviour

    switch (num) {
        ERROR_MSG(ERROR_CASE)
    }

    return "Unknown error";
}