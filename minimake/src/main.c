#include <err.h>
#include <stdio.h>

#include "build.h"
#include "makefile.h"
#include "option.h"
#include "parsing.h"
#include "print.h"

static char *choose_makefile(struct options *options)
{
    if (options->filename)
    {
        return options->filename;
    }
    char *filename = find_default_makefile();
    if (!filename && options->target_count == 0)
    {
        errx(2, "*** No targets specified and no makefile found.  Stop.");
    }
    if (!filename)
    {
        errx(2, "*** No rule to make target '%s'.  Stop.", options->targets[0]);
    }
    return filename;
}

static void run_makefile(struct options *options, char *filename)
{
    struct makefile *makefile = init_makefile();
    parse_makefile(makefile, filename);
    if (options->print)
    {
        beautyprint(makefile);
    }
    else
    {
        build_targets(makefile, options->targets, options->target_count);
    }
    free_makefile(makefile);
}

int main(int argc, char **argv)
{
    if (has_empty_argument(argc, argv))
    {
        errx(2, "*** empty string invalid as argument.  Stop.");
    }
    struct options options;
    parse_options(&options, argc, argv);
    if (options.invalid)
    {
        print_help(stderr);
        return 2;
    }
    if (options.help)
    {
        print_help(stdout);
        return 0;
    }
    char *filename = choose_makefile(&options);
    run_makefile(&options, filename);
    return 0;
}
