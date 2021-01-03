/************************************************
 *
 *  Author(s): Pedro Tavares
 *  Created Date: 24 Dec 2020
 *  Updated Date: 01 Jan 2021
 *
 ***********************************************/


#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "errors.h"

typedef struct {
    pthread_t prev_thread;
    uintptr_t (* process)(void *);
    uintptr_t (* write)(void *, _modules_error, _modules_error);
    void * args;
} Data;


// This static global variable is only accessed by the main thread
// `multithread[_create | _wait]`'s functions are Thread-Unsafe because of it. For Thread-Safety it requires a reentrant version
// For further examples check: `strtok` and `strtok_r` respectively defined in C standard and Posix only
// Why? Use same multithread[_create | _wait]'s function assignature for Windows and Posix
static pthread_t THREAD = 0;


static uintptr_t wrapper(Data * data)
{
    pthread_t prev_thread;
    uintptr_t prev_error = 0;
    int error;

    error = (*(data->process))(data->args);
    prev_thread = data->prev_thread;

    // In case of error in `data->process` it still joins the thread for resource cleanup
    if (prev_thread && pthread_join(prev_thread, (void **) &prev_error))
        prev_error = _THREAD_TERMINATION_FAILED;
    
    error = (*(data->write))(data->args, prev_error, error);
    
    free(data);

    return prev_error ? prev_error : error;
}


// Attention: Be careful that sizeof(void *) must be >= sizeof(_modules_error) 
// otherwise make sure the value returned by those funtions are always <= sizeof(void *)
// Why? Because we use a hack to consider a pointer as an _modules_error to pass between threads
_modules_error multithread_create(_modules_error (* process)(void *), _modules_error (* write)(void *, _modules_error, _modules_error), void * args)
{
    Data * data = malloc(sizeof(Data));

    * data = (Data) {
        .prev_thread = THREAD,
        .process = (uintptr_t (*) (void *)) process,
        .write = (uintptr_t (*) (void *, _modules_error, _modules_error)) write,
        .args = args,
    };
    
    if (pthread_create(&THREAD, NULL, (void *) wrapper, data)) {
        free(data);
        return _THREAD_CREATION_FAILED;
    }

    return _SUCCESS;
}

_modules_error multithread_wait()
{
    uintptr_t error;

    if (pthread_join(THREAD, (void **) &error))
        return _THREAD_TERMINATION_FAILED;
    
    return error;
}

/*

    Test Code


typedef struct {
    int index;
} Arguments;

#include <stdio.h>

int func(void * data)
{
    Arguments * args = (Arguments * ) data;
    sleep(rand() % 60);
    printf("Process %d\n", args->index);
    return 2;
}

int func2(void * data, int prev_error, int error)
{
    Arguments * args = (Arguments * ) data;
    printf("Write %d\n", args->index);
    return prev_error ? prev_error : 3;
}



int main()
{
    printf("%d\n", k);
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < 50; ++i) {
        Arguments * a_p = malloc(sizeof(Arguments));
        *a_p = (Arguments) {.index=i};
        multithread_create(func, func2, a_p);
    }

    int error = 0;
    puts("FINISHED DEPLOYING THREADS");
    

    error = multithread_wait();

    puts("FINISHED ALL THREADS");

    if (error)
        printf("Error: %d\n", error);

    return 0;
}

*/
