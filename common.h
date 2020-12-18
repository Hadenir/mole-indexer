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

typedef struct mole_context
{
    char* path_d;
    char* path_f;
    int time;
    mole_index_t* index;
    pthread_mutex_t* index_mutex;
    bool indexing_pending;
    pthread_t worker_tid;
    bool force_exit;
} mole_context_t;

// Prints usage message to stderr and terminates the program.
void usage(char* name);
