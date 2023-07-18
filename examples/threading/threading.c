#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // DONE: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    int ret;

    usleep(thread_func_args->wait_to_obtain_ms);
    ret = pthread_mutex_lock(thread_func_args->lock);

    if (ret == 0) {
        thread_func_args->thread_complete_success = true;
        usleep(thread_func_args->wait_to_release_ms);
        pthread_mutex_unlock(thread_func_args->lock);
    }

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * DONE: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    int ret;

    struct thread_data *td = (struct thread_data *)malloc(sizeof(struct thread_data));

    memset(td, 0, sizeof(struct thread_data));
    td->wait_to_obtain_ms = wait_to_obtain_ms;
    td->wait_to_release_ms = wait_to_release_ms;
    td->lock = mutex;
    td->thread_complete_success = false;

    ret = pthread_create(thread, NULL, threadfunc, td);
    if (ret != 0) {
        return false;
    }

    return true;
}

