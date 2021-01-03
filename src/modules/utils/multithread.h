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

// This function can't count two elapsed times at both time.
float clock_main_thread(CLOCK_ACTION action);


_modules_error multithread_create(_modules_error (* process)(void *), _modules_error (* write)(void *, _modules_error, _modules_error), void * args);

_modules_error multithread_wait();

#endif //UTILS_MULTITHREAD_H