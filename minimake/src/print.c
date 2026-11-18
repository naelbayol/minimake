#include "print.h"

#include <assert.h>
#include <stdio.h>

static void print_variables(struct variable *var)
{
    while (var)
    {
        printf("'%s' = '%s'\n", var->name, var->value);
        var = var->next;
    }
}

static void print_rule(struct rule *rule)
{
    printf("(%s):", rule->target);
    for (size_t i = 0; i < rule->dependencies.count; i++)
    {
        printf(" [%s]", rule->dependencies.items[i]);
    }
    printf("\n");
    for (size_t i = 0; i < rule->commands.count; i++)
    {
        printf("    '%s'\n", rule->commands.items[i]);
    }
}

void beautyprint(struct makefile *makefile)
{
    assert(makefile != NULL);
    printf("# variables\n");
    print_variables(makefile->variables);
    printf("# rules\n");
    struct rule *rule = makefile->rules;
    while (rule)
    {
        print_rule(rule);
        rule = rule->next;
    }
}
