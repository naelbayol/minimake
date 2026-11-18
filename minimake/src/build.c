#include "build.h"

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "exec.h"

struct file_info
{
    int exists;
    struct timespec mtime;
};

static void get_file_info(char *path, struct file_info *info)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        info->exists = 0;
        return;
    }
    info->exists = 1;
    info->mtime = st.st_mtim;
}

static int is_newer(struct timespec *first, struct timespec *second)
{
    if (first->tv_sec != second->tv_sec)
    {
        return first->tv_sec > second->tv_sec;
    }
    return first->tv_nsec > second->tv_nsec;
}

static void build_rule(struct build_context *context, struct rule *rule);

static void build_dependency(struct build_context *context, struct rule *rule,
                             char *dependency)
{
    struct rule *dep_rule = find_rule(context->makefile, dependency);
    if (dep_rule)
    {
        build_rule(context, dep_rule);
        return;
    }
    struct file_info info;
    get_file_info(dependency, &info);
    if (!info.exists)
    {
        errx(2, "*** No rule to make target '%s', needed by '%s'.  Stop.",
             dependency, rule->target);
    }
}

static int dependency_is_newer(struct build_context *context, char *dependency,
                               struct file_info *target)
{
    struct rule *dep_rule = find_rule(context->makefile, dependency);
    if (dep_rule && dep_rule->rebuilt)
    {
        return 1;
    }
    struct file_info info;
    get_file_info(dependency, &info);
    return target->exists && info.exists
        && is_newer(&info.mtime, &target->mtime);
}

static int needs_rebuild(struct build_context *context, struct rule *rule,
                         struct file_info *target)
{
    if (!target->exists && rule->commands.count > 0)
    {
        return 1;
    }
    for (size_t i = 0; i < rule->dependencies.count; i++)
    {
        if (dependency_is_newer(context, rule->dependencies.items[i], target))
        {
            return 1;
        }
    }
    return 0;
}

static void build_rule(struct build_context *context, struct rule *rule)
{
    if (rule->state == RULE_DONE)
    {
        return;
    }
    rule->state = RULE_IN_PROGRESS;
    for (size_t i = 0; i < rule->dependencies.count; i++)
    {
        build_dependency(context, rule, rule->dependencies.items[i]);
    }
    struct file_info target;
    get_file_info(rule->target, &target);
    if (needs_rebuild(context, rule, &target))
    {
        execute_rule(context, rule);
        rule->rebuilt = 1;
    }
    rule->state = RULE_DONE;
}

static void build_target(struct build_context *context, char *target)
{
    struct rule *rule = find_rule(context->makefile, target);
    if (!rule)
    {
        errx(2, "*** No rule to make target '%s'.  Stop.", target);
    }
    size_t before = context->executed;
    build_rule(context, rule);
    if (context->executed == before && rule->commands.count == 0)
    {
        printf("minimake: Nothing to be done for '%s'.\n", target);
    }
    else if (context->executed == before)
    {
        printf("minimake: '%s' is up to date.\n", target);
    }
}

static struct rule *default_rule(struct makefile *makefile)
{
    struct rule *rule = makefile->rules;
    while (rule && strchr(rule->target, '%'))
    {
        rule = rule->next;
    }
    return rule;
}

void build_targets(struct makefile *makefile, char **targets, int count)
{
    assert(makefile != NULL);
    assert(count == 0 || targets != NULL);
    struct build_context context = { .makefile = makefile, .executed = 0 };
    if (count == 0)
    {
        struct rule *rule = default_rule(makefile);
        if (!rule)
        {
            errx(2, "*** No targets.  Stop.");
        }
        build_target(&context, rule->target);
        return;
    }
    for (int i = 0; i < count; i++)
    {
        build_target(&context, targets[i]);
    }
}
