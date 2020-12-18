/*
 * ---------------------------------------------------------------------------------
 * Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągniecia efektów
 * uczenia się z przedmiotu SOP1 została wykonana przeze mnie samodzielnie.
 *
 * Konrad Brzózka 305866
 * ---------------------------------------------------------------------------------
*/

// TODO:
// - maximum path and file_name lengths
// - periodic indexing

#include "common.h"
#include "mole_index.h"
#include "indexer.h"
#include "cli.h"

#define TIME_MIN 30
#define TIME_MAX 7200

// Parses command arguments and checks if provided values are correct.
// Uses default values if necessary (e.g. environment variables).
void parseargs(int argc, char** argv, char** path_d, char** path_f, int* time)
{
    int opt;

    *path_d = NULL;
    *path_f = NULL;
    *time = -1;

    opterr = 0;
    while((opt = getopt(argc, argv, "hd:f:t:")) != -1)
    {
        switch(opt)
        {
            case 'h':
                usage(argv[0]);
            break;
            case 'd':
                *path_d = optarg;
            break;
            case 'f':
                *path_f = optarg;
            break;
            case 't':
                *time = atoi(optarg);
                if(*time < TIME_MIN || *time > TIME_MAX)
                    usage(argv[0]);
            break;
            case '?':
                usage(argv[0]);
            break;
            default:
                ERROR("getopt");
            break;
        }
    }

    if(argc > optind) usage(argv[0]);

    if(NULL == *path_d)
    {
        if(!(*path_d = getenv(MOLE_DIR_VAR))) usage(argv[0]);
    }

    if(NULL == *path_f)
    {
        if(!(*path_f = getenv(MOLE_INDEX_PATH_VAR)))
        {
            static char home[PATH_MAX];
            strcpy(home, getenv("HOME"));
            strcat(home, MOLE_INDEX_NAME_DEFAULT);

            *path_f = home;
        }
    }
}

int main(int argc, char** argv)
{
    char* path_d;
    char* path_f;
    int time;
    parseargs(argc, argv, &path_d, &path_f, &time);

    pthread_mutex_t index_mutex = PTHREAD_MUTEX_INITIALIZER;

    mole_index_t index;
    index_init(&index);

    mole_context_t context;
    context.path_d = path_d;
    context.path_f = path_f;
    context.time = time;
    context.index = &index;
    context.index_mutex = &index_mutex;
    context.indexing_pending = false;
    context.force_exit = false;

    if(!index_read(&index, path_f))
    {
        indexer_start_worker(&context);
    }

    cli_start(&context);

    index_free(&index);

    return EXIT_SUCCESS;
}
