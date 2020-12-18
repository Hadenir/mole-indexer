#include "common.h"

void usage(char* name)
{
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h          \tPrint this message.\n");
    fprintf(stderr, "  -d <path>   \tBrowse <path> directory. If not specified uses path\n");
    fprintf(stderr, "              \tin $MOLDE_DIR environment variable (required).\n");
    fprintf(stderr, "  -f <file>   \tRead/Write index cache from/into <file>. If not specified\n");
    fprintf(stderr, "              \tuses file in $MOLE_INDEX_PATH. If this is also missing,\n");
    fprintf(stderr, "              \tdefaults to ~/.mole-index file.\n");
    fprintf(stderr, "  -t <arg>    \tSet time between performing periodic indexing to <arg> seconds.\n");
    fprintf(stderr, "              \tValue of <arg> has to be a number from interval [30, 7200].\n");
    fprintf(stderr, "              \tIf not specified, indexing is executed only once.\n");
    exit(EXIT_FAILURE);
}
