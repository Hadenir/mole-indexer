#pragma once

#include "common.h"
#include "mole_index.h"

#define COMMAND_MAX 16
#define PAGER_VAR "PAGER"

// Precalculated hashes of available commands.
#define HELP        0x00684d4018ed0681
#define EXIT        0x00654b1b96b0ba1e
#define EXIT_FORCE  0x6409127acf9bcd83
#define INDEX       0x67f07f670b83d132
#define COUNT       0x620751b5aae871af
#define LARGER_THAN 0x04ad3909ddf4b578
#define NAME_PART   0x6bce8a0f0036f21e
#define OWNER       0x6de3b4974ab7fcf3

typedef size_t hash_t;

// Starts command line interface. Begins waiting for command input.
void cli_start(mole_context_t* context);

// Prints command prompt and parses user input.
void cli_prompt(char* command, char* arg);

// These functions print error messages:
void cli_unrecognized_cmd(const char* command);
void cli_missing_param(const char* command);

// Handler functions for all CLI commands:
void cli_help();
void cli_index(mole_context_t* context);
void cli_count(mole_context_t* context);
void cli_largerthan(mole_context_t* context, size_t size);
void cli_namepart(mole_context_t* context, const char* string);
void cli_owner(mole_context_t* context, uid_t uid);

// Prints contents of index (full path, size, file type)
void cli_print_index(const mole_index_t* index);

// Helper function for getting letter representing file type to print.
char cli_get_type_letter(file_type_t type);

// Implementation of sdbm, simple string hashing algorithm.
// It will do sufficiently for the purpose of matching commands.
hash_t cli_hash(const char* string);
