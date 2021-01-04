#ifndef UTILS_MULTITHREAD_H
#define UTILS_MULTITHREAD_H

#include <stdbool.h>

#include "errors.h"

#ifdef _WIN32
#define THREADS
#define WIN_THREADS

#elif defined(__unix__) || defined(__APPLE__) || defined(__MACH__)
#define THREADS
#define POSIX_THREADS

#else
#define _NO_MULTITHREAD // Avoid explicit if-statements if it is defined at compile time

#endif

extern bool NO_MULTITHREAD;

typedef enum {STOP_CLOCK, START_CLOCK} CLOCK_ACTION;

/**
\brief Counts the CPU time for the current thread in order to avoid the sum of each thread's CPU time elapsed
 Warning: This function can't count two elapsed times at both time.
 @param path Pointer to the SHAFA->RLE file's path
 @param decompress_rle Decompresses file with RLE's algorithm too
 @returns Error status
*/
float clock_main_thread(CLOCK_ACTION action);

/**
\brief Calls both functions in a separated thread. Even though its a different process, write's function will execute sequentially.
 Warning: This function isn't thread-safe itself
 @param process This is the processing function which doesn't do IO sequencially
 @param write This is the function which does IO sequentially
 @param args Arguments passed to both other parameters of this multithread_create's function
 @returns Error status
*/
_modules_error multithread_create(_modules_error (* process)(void *), _modules_error (* write)(void *, _modules_error, _modules_error), void * args);

/**
\brief Waits for all threads created from multithread_create's function
 Warning: This function isn't thread-safe itself
 @returns Error status
*/
_modules_error multithread_wait();

#endif //UTILS_MULTITHREAD_H
