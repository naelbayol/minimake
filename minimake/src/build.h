#ifndef BUILD_H
#define BUILD_H

#include <stddef.h>

#include "makefile.h"

struct build_context
{
    struct makefile *makefile;
    size_t executed;
};

void build_targets(struct makefile *makefile, char **targets, int count);

#endif /* ! BUILD_H */
