#include "option.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int has_empty_argument(int argc, char **argv)
{
    assert(argv != NULL);
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '\0')
        {
            return 1;
        }
    }
    return 0;
}

static void init_options(struct options *options, char **argv)
{
    options->filename = NULL;
    options->print = 0;
    options->help = 0;
    options->invalid = 0;
    options->targets = argv + 1;
    options->target_count = 0;
}

static int read_next_filename(struct options *options, int argc, char **argv,
                              int i)
{
    if (i + 1 >= argc)
    {
        options->invalid = 1;
        return i + 1;
    }
    options->filename = argv[i + 1];
    return i + 2;
}

static int handle_argument(struct options *options, int argc, char **argv,
                           int i)
{
    char *arg = argv[i];
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
    {
        options->help = 1;
    }
    else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--print-data-base") == 0
             || strcmp(arg, "--pretty-print") == 0)
    {
        options->print = 1;
    }
    else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--file") == 0
             || strcmp(arg, "--makefile") == 0)
    {
        return read_next_filename(options, argc, argv, i);
    }
    else if (strncmp(arg, "--file=", 7) == 0)
    {
        options->filename = arg + 7;
    }
    else if (strncmp(arg, "--makefile=", 11) == 0)
    {
        options->filename = arg + 11;
    }
    else if (arg[0] == '-')
    {
        options->invalid = 1;
    }
    else
    {
        options->targets[options->target_count] = arg;
        options->target_count++;
    }
    return i + 1;
}

void parse_options(struct options *options, int argc, char **argv)
{
    assert(options != NULL);
    assert(argv != NULL);
    init_options(options, argv);
    int i = 1;
    while (i < argc)
    {
        i = handle_argument(options, argc, argv, i);
    }
}

void print_help(FILE *stream)
{
    assert(stream != NULL);
    fprintf(stream, "Usage: minimake [options] [target] ...\n");
    fprintf(stream, "Options:\n");
    fprintf(stream, "  -f FILE, --file=FILE, --makefile=FILE\n");
    fprintf(stream, "      Read FILE as a makefile.\n");
    fprintf(stream, "  -h, --help\n");
    fprintf(stream, "      Print this message and exit.\n");
    fprintf(stream, "  -p, --print-data-base, --pretty-print\n");
    fprintf(stream, "      Print the parsed makefile without running it.\n");
}

char *find_default_makefile(void)
{
    if (access("makefile", F_OK) == 0)
    {
        return "makefile";
    }
    if (access("Makefile", F_OK) == 0)
    {
        return "Makefile";
    }
    return NULL;
}
