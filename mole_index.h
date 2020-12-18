#pragma once

#include "common.h"

#define MOLE_DIR_VAR "MOLE_DIR"
#define MOLE_INDEX_PATH_VAR "MOLE_INDEX_PATH"
#define MOLE_INDEX_NAME_DEFAULT "/.mole-index"
#define MOLE_DEFAULT_CAPACITY 20
#define DEFAULT_MASK 0644

// Program scans for there types of files.
typedef enum file_type
{
    Unrecognized,  // Other file types
    Directory,          // Ordinary directory
    Image_JPEG,         // JPEG image
    Image_PNG,          // PNG image
    Compressed_GZIP,    // File compressed using gzip
    Compressed_ZIP      // File compressed using zip (including format such as .docx, .odt, etc.)
} file_type_t;

// Represents single entry inside program's index.
typedef struct mole_index_entry
{
    char file_name[STR_MAX];
    char full_path[PATH_MAX];
    size_t size;
    uid_t owner_uid;
    file_type_t file_type;
} mole_index_entry_t;

// Index is stored as a single dynamic array. The array is usually bigger than it needs.
// When inserting new elements, it eventually becomes full. If this happens,
// new array is created (twice the size) and the elements are copied over.
//
// I chose this data structure, because it's really easy to save to a file. Program needs
// to browse the index using various criteria (name, size, owner), so it doesn't make sense
// to sort the elements in regard to only one attribute (e.g. size).
//
// This implementation is inspired by std::vector.
typedef struct mole_index
{
    size_t size;                    // Number of entries currently in the index.
    size_t capacity;                // True size of the array.
    mole_index_entry_t* elements;   // Dynamic array of entries.
} mole_index_t;

// Initializes index to its default state.
void index_init(mole_index_t* index);

// Initializes index, preparing it to hold `capacity` elements without reallocating.
void index_init_capacity(mole_index_t* index, size_t capacity);

// Frees memory used by index.
void index_free(mole_index_t* index);

// Changes capacity of index to hold `new_capacity` elements.
// Reallocates internal array, if necessary.
void index_extend(mole_index_t* index, size_t new_capacity);

// Inserts new entry to the index.
void index_insert(mole_index_t* index, mole_index_entry_t* entry);

// Constructs new entry using provided values and inserts it to the index.
void index_emplace(mole_index_t* index, const char* file_name, const char* full_path,
                   size_t size, uid_t owner_uid, file_type_t file_type);

// Clears index, leaving its capacity unchanged.
void index_clear(mole_index_t* index);

// Reads index from file.
bool index_read(mole_index_t* index, char* index_path);

// Saves index into file.
void index_save(mole_index_t* index, char* index_path);
