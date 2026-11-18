#ifndef EXEC_H
#define EXEC_H

#include "build.h"
#include "makefile.h"

void execute_rule(struct build_context *context, struct rule *rule);

#endif /* ! EXEC_H */
