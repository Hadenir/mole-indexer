#pragma once

#define _GNU_SOURCE
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define ERROR(source) (fprintf(stderr,"(%s:%d) ",__FILE__,__LINE__),perror(source),exit(EXIT_FAILURE))

#define STR_MAX 256
#define PATH_MAX 4096

typedef struct mole_index mole_index_t;

// This struct holds all necessary information for the program.
// Those values are used all over the code, that's why we keep them
// in a single struct, that we can pass everywhere it is needed.
typedef struct mole_context
{
    char* path_d;                           // Path to directory, root of indexing operations
    char* path_f;                           // Path to cache file where indexing results are stored
    int time;                               // Time between periodic indexing
    mole_index_t* index;                    // Pointer to index
    pthread_mutex_t* index_mutex;           // Mutex guarding acces to index
    bool indexing_pending;                  // Flag telling wheter there is indexing process pending
    pthread_cond_t* indexing_done;          // Condtion variable that is signaled when indexing is done
    pthread_mutex_t* indexing_mutex;        // Mutex guarding both flag and condition variable
    bool force_exit;                        // Flag that can be set to interrupt indexing process
    pthread_mutex_t* force_exit_mutex;      // Mutex guarding access to above flag
} mole_context_t;

// Prints usage message to stderr and terminates the program.
void usage(char* name);
