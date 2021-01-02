#ifndef UTILS_MULTITHREAD_H
#define UTILS_MULTITHREAD_H

_modules_error multithread_create(_modules_error (* process)(void *), _modules_error (* write)(void *, _modules_error, _modules_error), void * args);

_modules_error multithread_wait();

#endif //UTILS_MULTITHREAD_H