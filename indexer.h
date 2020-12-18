#pragma once

#include <pthread.h>

#include "common.h"
#include "mole_index.h"

file_type_t get_file_type(size_t signature);

void indexer_start_worker(mole_context_t* context);

void* indexer_worker(void* args);
