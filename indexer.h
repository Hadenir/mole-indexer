#pragma once

#include <pthread.h>

#include "common.h"
#include "mole_index.h"

// Compares provided `signature` (first 64 bits of a file) and
// returns what file type it is.
file_type_t get_file_type(uint64_t signature);

// Checks whether there is already a worker running.
// If not, launches one in a seperate thread.
void indexer_start_worker(mole_context_t* context);

// This function is supposed to be executed inside
// seperate thread. See `indexer_start_worker()`.
void* indexer_worker(void* args);
