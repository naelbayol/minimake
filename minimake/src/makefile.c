#include "makefile.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

struct makefile *init_makefile(void)
{
    struct makefile *m = malloc(sizeof(struct makefile));
    check_allocation(m, "malloc on init_makefile.");
    m->rules = NULL;
    m->last_rule = NULL;
    m->variables = NULL;
    m->last_variable = NULL;
    return m;
}

void free_makefile(struct makefile *makefile)
{
    if (makefile)
    {
        struct rule *current_rule = makefile->rules;
        while (current_rule)
        {
            struct rule *next_rule = current_rule->next;
            free(current_rule->target);
            free_string_list(&current_rule->dependencies);
            free_string_list(&current_rule->commands);
            free(current_rule);
            current_rule = next_rule;
        }
        struct variable *current_var = makefile->variables;
        while (current_var)
        {
            struct variable *next_var = current_var->next;
            free(current_var->name);
            free(current_var->value);
            free(current_var);
            current_var = next_var;
        }
        free(makefile);
    }
}

struct variable *find_variable(struct makefile *makefile, char *name)
{
    assert(makefile != NULL);
    assert(name != NULL);
    struct variable *current = makefile->variables;
    while (current)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct rule *find_rule(struct makefile *makefile, char *target)
{
    assert(makefile != NULL);
    assert(target != NULL);
    struct rule *current = makefile->rules;
    while (current)
    {
        if (strcmp(current->target, target) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct variable *add_variable(struct makefile *makefile, char *name,
                              char *value)
{
    assert(makefile != NULL);
    assert(name != NULL);
    assert(value != NULL);
    struct variable *var = find_variable(makefile, name);
    if (var)
    {
        char *temp = var->value;
        var->value = strdup(value);
        free(temp);
        check_allocation(var->value,
                         "strdup for replacing variable on add_variable.");
        return var;
    }
    struct variable *new_var = malloc(sizeof(struct variable));
    check_allocation(new_var, "malloc for new variable on add_variable.");
    new_var->name = strdup(name);
    check_allocation(new_var->name,
                     "strdup name for new variable on add_variable.");
    new_var->value = strdup(value);
    check_allocation(new_var->value,
                     "strdup value for new variable on add_variable.");
    new_var->next = NULL;
    if (makefile->last_variable)
    {
        makefile->last_variable->next = new_var;
    }
    else
    {
        makefile->variables = new_var;
    }
    makefile->last_variable = new_var;
    return new_var;
}

struct rule *add_rule(struct makefile *makefile, char *target)
{
    assert(makefile != NULL);
    assert(target != NULL);
    struct rule *new_rule = malloc(sizeof(struct rule));
    check_allocation(new_rule, "malloc for new rule on add_rule.");
    new_rule->target = strdup(target);
    check_allocation(new_rule->target,
                     "strdup for target to new rule on add_rule.");
    init_string_list(&new_rule->dependencies);
    init_string_list(&new_rule->commands);
    new_rule->state = RULE_NOT_VISITED;
    new_rule->rebuilt = 0;
    new_rule->next = NULL;
    if (makefile->last_rule)
    {
        makefile->last_rule->next = new_rule;
    }
    else
    {
        makefile->rules = new_rule;
    }
    makefile->last_rule = new_rule;
    return new_rule;
}
