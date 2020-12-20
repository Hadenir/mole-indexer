#include <fcntl.h>

#include "mole_index.h"

void index_init_capacity(mole_index_t* index, size_t capacity)
{
    index->size = 0;
    index->capacity = capacity;
    index->elements = malloc(index->capacity * sizeof(mole_index_entry_t));
    if(NULL == index->elements) ERROR("malloc");

    memset(index->elements, 0, index->capacity * sizeof(mole_index_entry_t));
}

void index_init(mole_index_t* index)
{
    index_init_capacity(index, MOLE_DEFAULT_CAPACITY);
}

void index_free(mole_index_t* index)
{
    index->size = 0;
    index->capacity = 0;
    free(index->elements);
    index->elements = NULL;
}

void index_extend(mole_index_t* index, size_t new_capacity)
{
    if(new_capacity <= index->capacity) return;

    mole_index_entry_t* new_elements = malloc(new_capacity * sizeof(mole_index_entry_t));
    if(NULL == new_elements) ERROR("malloc");

    memcpy(new_elements, index->elements, index->size * sizeof(mole_index_entry_t));
    free(index->elements);
    index->capacity = new_capacity;
    index->elements = new_elements;
}

void index_insert(mole_index_t* index, mole_index_entry_t* entry)
{
    if(index->size >= index->capacity) index_extend(index, index->capacity * 2);

    size_t i = index->size;
    index->elements[i] = *entry;
    index->size++;
}

void index_emplace(mole_index_t* index, const char* filename, const char* full_path,
                   size_t size, uid_t owner_uid, file_type_t file_type)
{
    mole_index_entry_t entry;
    strncpy(entry.file_name, filename, STR_MAX);
    if(realpath(full_path, entry.full_path) == NULL) ERROR("realpath");
    entry.size = size;
    entry.owner_uid = owner_uid;
    entry.file_type = file_type;

    index_insert(index, &entry);
}

void index_clear(mole_index_t* index)
{
    index->size = 0;
    memset(index->elements, 0, index->capacity * sizeof(mole_index_entry_t));
}

bool index_read(mole_index_t* index, char* index_path)
{
    int fd = open(index_path, O_RDONLY);
    if(fd < 0)
    {
        if(errno == ENOENT) return false;
        else ERROR("open");
    }

    uint64_t index_size;
    if(read(fd, &index_size, sizeof(index_size)) < 0)  ERROR("read");

    index_extend(index, index_size);

    if(read(fd, index->elements, index_size * sizeof(mole_index_entry_t)) < 0) ERROR("read");

    index->size = index_size;

    close(fd);

    return true;
}

void index_save(mole_index_t* index, char* index_path)
{
    int fd = open(index_path, O_CREAT | O_WRONLY, DEFAULT_MASK);
    if(fd < 0) ERROR("open");

    uint64_t index_size = index->size;
    if(write(fd, &index_size, sizeof(index_size)) < 0) ERROR("write");

    if(write(fd, index->elements, index_size * sizeof(mole_index_entry_t)) < 0) ERROR("write");

    close(fd);
}
