#include <pthread.h>

#include "cli.h"
#include "indexer.h"

void cli_start(mole_context_t* context)
{
    char command[COMMAND_MAX];
    char argument[STR_MAX];

    bool exit = false;
    do
    {
        cli_prompt(command, argument);

        // printf("CMD: %s\t ARG: %s\n", command, arg);
        bool arg_present = strlen(argument) > 0;

        switch(cli_hash(command))
        {
            case HELP:
                cli_help();
            break;
            case EXIT_FORCE:
                context->force_exit = true;
            case EXIT:
                printf("Exiting...\n");
                exit = true;
                while(context->indexing_pending)
                    sleep(1);
                printf("Goodbye!\n");
            break;
            case INDEX:
                cli_index(context);
            break;
            case COUNT:
                cli_count(context);
            break;
            case LARGER_THAN:
                if(!arg_present)
                    cli_missing_param(command);
                else
                    cli_largerthan(context, atoll(argument));
            break;
            case NAME_PART:
                if(!arg_present)
                    cli_missing_param(command);
                else
                    cli_namepart(context, argument);
            break;
            case OWNER:
                if(!arg_present)
                    cli_missing_param(command);
                else
                    cli_owner(context, atoi(argument));
            break;
            default:
                cli_unrecognized_cmd(command);
                continue;
        }
    }
    while(!exit);
}

void cli_prompt(char* command, char* arg)
{
    memset(command, 0, COMMAND_MAX);
    memset(arg, 0, STR_MAX);

    char line[STR_MAX];

    printf("> "); fflush(stdin);
    fgets(line, STR_MAX, stdin);
    if(sscanf(line, " %15s %255[._-0-9a-zA-Z ]", command, arg) == EOF)
        ERROR("scanf");
}

void cli_unrecognized_cmd(const char* command)
{
    fprintf(stderr, "Unrecognized command: `%s`. Type `help` to see list of available commands.\n", command);
}

void cli_missing_param(const char* command)
{
    fprintf(stderr, "Missing parameter for command `%s`.\n", command);
}

void cli_help()
{
    printf("Available commands:\n");
    printf("  help             \tPrints this message.\n");
    printf("  exit             \tTries to exit the program. If there are background\n");
    printf("                   \ttasks in progress, waits for them to finish.\n");
    printf("  exit!            \tForces program to quit. Background tasks are interrupted,\n");
    printf("                   \tbut saving the index into a file is guaranteed to complete.\n");
    printf("  index            \tStarts background indexing of specified directory (if it\n");
    printf("                   \tis not already being performed).\n");
    printf("  count            \tPrepares summary of how many files of each type are there\n");
    printf("                   \tin the index.\n");
    printf("  largerthan <size>\tPrints all files in the index that are larger than <size> bytes.\n");
    printf("  namepart <string>\tPrints all files in the index that contain <string> inside their name.\n");
    printf("  owner <uid>      \tPrints all files in the index whose owner is user with id <uid>.\n");
}

void cli_index(mole_context_t* context)
{
    if(context->indexing_pending)
    {
        printf("Indexing is already pending!\n");
        return;
    }

    printf("Starting indexing process...\n");
    indexer_start_worker(context);
}

void cli_count(mole_context_t* context)
{
    printf("Counting files...\n");

    size_t directories = 0, jpegs = 0, pngs = 0, gzips = 0, zips = 0;
    pthread_mutex_lock(context->index_mutex);
    for(size_t i = 0; i < context->index->size; ++i)
    {
        switch(context->index->elements[i].file_type)
        {
            case Directory: directories++; break;
            case Image_JPEG: jpegs++; break;
            case Image_PNG: pngs++; break;
            case Compressed_GZIP: gzips++; break;
            case Compressed_ZIP: zips++; break;
            default: break;
        }
    }
    pthread_mutex_unlock(context->index_mutex);

    printf("Done!\n");

    printf("File Count Summary:\n");
    printf("  Directories: %ld\n", directories);
    printf("  JPEG Images: %ld\n", jpegs);
    printf("  PNG Images: %ld\n", pngs);
    printf("  GZIP Compressed Files: %ld\n", gzips);
    printf("  ZIP Compressed Files: %ld\n", zips);
}

void cli_largerthan(mole_context_t* context, size_t size)
{
    printf("Looking for files larger than %ld bytes...\n", size);

    mole_index_t result;
    index_init(&result);

    pthread_mutex_lock(context->index_mutex);
    for(size_t i = 0; i < context->index->size; ++i)
    {
        if(context->index->elements[i].size > size)
            index_insert(&result, &context->index->elements[i]);
    }
    pthread_mutex_unlock(context->index_mutex);

    printf("Done!\n");
    cli_print_index(&result);

    index_free(&result);
}

void cli_namepart(mole_context_t* context, const char* string)
{
    printf("Looking for files whose names contain \"%s\"...\n", string);

    mole_index_t result;
    index_init(&result);

    pthread_mutex_lock(context->index_mutex);
    for(size_t i = 0; i < context->index->size; ++i)
    {
        if(strstr(context->index->elements[i].file_name, string) != NULL)
            index_insert(&result, &context->index->elements[i]);
    }
    pthread_mutex_unlock(context->index_mutex);

    printf("Done!\n");
    cli_print_index(&result);

    index_free(&result);
}

void cli_owner(mole_context_t* context, uid_t uid)
{
    printf("Looking for files of user with id %d...\n", uid);

    mole_index_t result;
    index_init(&result);

    pthread_mutex_lock(context->index_mutex);
    for(size_t i = 0; i < context->index->size; ++i)
    {
        if(context->index->elements[i].owner_uid == uid)
            index_insert(&result, &context->index->elements[i]);
    }
    pthread_mutex_unlock(context->index_mutex);

    printf("Done!\n");
    cli_print_index(&result);

    index_free(&result);
}

void cli_print_index(const mole_index_t* index)
{
    FILE* stream = stdout;

    if(index->size > 3)
    {
        const char* pager = getenv(PAGER_VAR);
        if(NULL != pager) stream = popen(pager, "w");
        if(NULL == stream) ERROR("popen");
    }

    fprintf(stream, "Types: d - Directory, j - JPEG image, p - PNG image\n");
    fprintf(stream, "       g - compressed GZIP file, z - compressed ZIP file\n\n");
    fprintf(stream, "Type\tSize\t\tPath\n");
    for(size_t i = 0; i < index->size; ++i)
    {
        char type_letter = cli_get_type_letter(index->elements[i].file_type);
        fprintf(stream, "%c\t%ld\t\t%s\n", type_letter, index->elements[i].size, index->elements[i].full_path);
    }

    fprintf(stream, "\n");

    if(stdout != stream)
        if(pclose(stream) != 0) ERROR("pclose");
}

char cli_get_type_letter(file_type_t type)
{
    switch(type)
    {
        case Directory: return 'd';
        case Image_JPEG: return 'j';
        case Image_PNG: return 'p';
        case Compressed_GZIP: return 'g';
        case Compressed_ZIP: return 'z';
        default: return '-';
    }
}

hash_t cli_hash(const char* string)
{
    hash_t hash = 0;
    char c;
    while((c = *string++))
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}
