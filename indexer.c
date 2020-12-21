#include "indexer.h"

#include <fcntl.h>
#include <fts.h>
#include <sys/stat.h>

file_type_t get_file_type(uint64_t signature)
{
    if((signature & 0xffffff) == 0xffd8ff)  // JPEG signature: FF D8 FF (...)
        return Image_JPEG;
    if(signature == 0x0a1a0a0d474e5089)     // PNG signature: 89 50 4E 47 0D 0A 1A 0A (.PNG....)
        return Image_PNG;
    if((signature & 0xffff) == 0x8b1f)      // GZIP signature: 1F 8B (..)
        return Compressed_GZIP;
    if((signature & 0xffff) == 0x4b50)      // ZIP signature: 50 4B (PK)
        return Compressed_ZIP;
    else
        return Unrecognized;
}

void indexer_start_worker(mole_context_t* context)
{
    pthread_mutex_lock(context->indexing_mutex);
    if(context->indexing_pending)
    {
        pthread_mutex_unlock(context->indexing_mutex);
        return;
    }

    context->indexing_pending = true;
    pthread_mutex_unlock(context->indexing_mutex);

    pthread_attr_t attributes;
    if(pthread_attr_init(&attributes)) ERROR("pthread_attr_init");
    if(pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED)) ERROR("pthread_attr_setdetachstate");

    if(pthread_create(&context->worker_tid, &attributes, indexer_worker, context)) ERROR("pthread_create");

    if(pthread_attr_destroy(&attributes)) ERROR("pthread_attr_destroy");
}

void* indexer_worker(void* args)
{
    mole_context_t* context = (mole_context_t*) args;

    mole_index_t new_index;
    index_init(&new_index);

    char* paths[] = {context->path_d, NULL};
    FTS* fts = fts_open(paths, FTS_LOGICAL | FTS_NOCHDIR, NULL);
    if(fts == NULL) ERROR("fts_open");

    FTSENT* entry;
    errno = 0;
    while((entry = fts_read(fts)) != NULL)
    {
        switch(entry->fts_info)
        {
            case FTS_D:
            {
                char* file_name = entry->fts_name;
                char* full_path = entry->fts_accpath;
                size_t size = entry->fts_statp->st_size;
                uid_t owner_uid = entry->fts_statp->st_uid;
                file_type_t type = Directory;

                index_emplace(&new_index, file_name, full_path, size, owner_uid, type);
            }
            break;
            case FTS_F:
            {
                int fd = open(entry->fts_accpath, O_RDONLY);
                if(fd < 0) ERROR("open");

                size_t signature;
                if(read(fd, &signature, sizeof(size_t)) < 0)  ERROR("read");

                if(close(fd)) ERROR("close");

                char* file_name = entry->fts_name;
                char* full_path = entry->fts_accpath;
                size_t size = entry->fts_statp->st_size;
                uid_t owner_uid = entry->fts_statp->st_uid;
                file_type_t type = get_file_type(signature);

                if(type != Unrecognized)
                    index_emplace(&new_index, file_name, full_path, size, owner_uid, type);
            }
            break;
            default:
                continue;
        }
        errno = 0;

        pthread_mutex_lock(context->force_exit_mutex);
        if(context->force_exit)
        {
            if(fts_close(fts)) ERROR("fts_close");
            pthread_mutex_lock(context->indexing_mutex);
            context->indexing_pending = false;
            pthread_mutex_unlock(context->indexing_mutex);
            pthread_cond_broadcast(context->indexing_done);
            return NULL;
        }
        pthread_mutex_unlock(context->force_exit_mutex);
    }

    if(errno != 0) ERROR("fts_read");

    if(fts_close(fts)) ERROR("fts_close");

    pthread_mutex_lock(context->index_mutex);

    index_free(context->index);
    *context->index = new_index;

    index_save(context->index, context->path_f);

    pthread_mutex_unlock(context->index_mutex);

    printf("\b\bBackground indexing finished!\n> ");
    fflush(stdout);

    pthread_mutex_lock(context->indexing_mutex);
    context->indexing_pending = false;
    pthread_mutex_unlock(context->indexing_mutex);

    pthread_cond_broadcast(context->indexing_done);

    return NULL;
}
