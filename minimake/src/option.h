#ifndef OPTION_H
#define OPTION_H

#include <stdio.h>

struct options
{
    char *filename;
    int print;
    int help;
    int invalid;
    char **targets;
    int target_count;
};

int has_empty_argument(int argc, char **argv);
void parse_options(struct options *options, int argc, char **argv);
void print_help(FILE *stream);
// Returns NULL when neither makefile nor Makefile exists.
char *find_default_makefile(void);

#endif /* ! OPTION_H */
