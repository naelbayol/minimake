#include "exec.h"

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "expand.h"

static char *skip_silent(char *command, int *silent)
{
    *silent = 0;
    if (command[0] != '@')
    {
        return command;
    }
    *silent = 1;
    command++;
    while (*command == ' ' || *command == '\t')
    {
        command++;
    }
    return command;
}

static void wait_child(struct rule *rule, pid_t pid)
{
    int status = 0;
    if (waitpid(pid, &status, 0) == -1)
    {
        err(2, "waitpid");
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        errx(2, "*** [%s] Error %d", rule->target, WEXITSTATUS(status));
    }
}

static void run_command(struct rule *rule, char *command)
{
    if (fflush(stdout) == EOF || fflush(stderr) == EOF)
    {
        err(2, "fflush");
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        err(2, "fork");
    }
    if (pid == 0)
    {
        char *args[] = { "sh", "-c", command, NULL };
        execv("/bin/sh", args);
        err(127, "execv on /bin/sh");
    }
    wait_child(rule, pid);
}

static void execute_command(struct build_context *context, struct rule *rule,
                            char *expanded)
{
    int silent = 0;
    char *command = skip_silent(expanded, &silent);
    if (!silent)
    {
        printf("%s\n", command);
    }
    context->executed++;
    run_command(rule, command);
}

void execute_rule(struct build_context *context, struct rule *rule)
{
    assert(context != NULL);
    assert(rule != NULL);
    for (size_t i = 0; i < rule->commands.count; i++)
    {
        char *expanded = expanser(context->makefile, rule->commands.items[i]);
        if (!expanded)
        {
            errx(2, "*** unterminated variable reference.  Stop.");
        }
        execute_command(context, rule, expanded);
        free(expanded);
    }
}
