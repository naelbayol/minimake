#ifndef MAKEFILE_H
#define MAKEFILE_H

#include "string_list.h"

struct variable
{
    char *name;
    char *value;
    struct variable *next;
};

enum rule_state
{
    RULE_NOT_VISITED,
    RULE_IN_PROGRESS,
    RULE_DONE,
};

struct rule
{
    char *target;
    struct string_list dependencies;
    struct string_list commands;
    enum rule_state state;
    int rebuilt;
    struct rule *next;
};

struct makefile
{
    struct rule *rules;
    struct rule *last_rule;
    struct variable *variables;
    struct variable *last_variable;
};

struct makefile *init_makefile(void);
void free_makefile(struct makefile *makefile);
// Returns NULL when the variable is not defined.
struct variable *find_variable(struct makefile *makefile, char *name);
struct variable *add_variable(struct makefile *makefile, char *name,
                              char *value);
// Returns NULL when no rule has this target.
struct rule *find_rule(struct makefile *makefile, char *target);
struct rule *add_rule(struct makefile *makefile, char *target);

#endif /* ! MAKEFILE_H */
