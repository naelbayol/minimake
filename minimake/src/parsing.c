#include "parsing.h"

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "expand.h"

struct parser
{
    struct makefile *makefile;
    struct rule *current_rule;
    int ignore_recipe;
    size_t line_number;
    char *filename;
};

enum line_type
{
    LINE_RULE,
    LINE_VARIABLE,
    LINE_UNKNOWN
};

static void parser_error(struct parser *parser, char *message)
{
    errx(2, "%s:%zu: *** %s.  Stop.", parser->filename, parser->line_number,
         message);
}

static int is_blank_char(char c)
{
    return c == ' ' || c == '\t';
}

static int is_blank_line(char *line)
{
    for (size_t i = 0; line[i] != '\0'; i++)
    {
        if (!is_blank_char(line[i]))
        {
            return 0;
        }
    }
    return 1;
}

static void remove_newline(char *line)
{
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
    {
        line[len - 1] = '\0';
    }
}

static void cut_comment(char *line)
{
    char *hash = strchr(line, '#');
    if (hash)
    {
        *hash = '\0';
    }
}

static char *skip_blanks(char *str)
{
    while (is_blank_char(*str))
    {
        str++;
    }
    return str;
}

static char *remove_blanc(char *str)
{
    char *start = skip_blanks(str);
    size_t len = strlen(start);
    while (len > 0 && is_blank_char(start[len - 1]))
    {
        len--;
    }
    start[len] = '\0';
    return start;
}

static enum line_type search_separ(char *line, size_t *pos)
{
    for (size_t i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == ':')
        {
            *pos = i;
            return LINE_RULE;
        }
        if (line[i] == '=')
        {
            *pos = i;
            return LINE_VARIABLE;
        }
    }
    return LINE_UNKNOWN;
}

static void parse_command(struct parser *parser, char *line)
{
    if (parser->ignore_recipe)
    {
        return;
    }
    if (!parser->current_rule)
    {
        parser_error(parser, "recipe commences before first target");
    }
    add_to_string_list(&parser->current_rule->commands, skip_blanks(line));
}

static void parse_dependencies(struct rule *rule, char *deps)
{
    size_t i = 0;
    while (deps[i] != '\0')
    {
        while (is_blank_char(deps[i]))
        {
            i++;
        }
        size_t start = i;
        while (deps[i] != '\0' && !is_blank_char(deps[i]))
        {
            i++;
        }
        if (i > start)
        {
            char *dep = strndup(deps + start, i - start);
            check_allocation(dep, "strndup for dependency on parse_rule.");
            add_to_string_list(&rule->dependencies, dep);
            free(dep);
        }
    }
}

static void parse_rule(struct parser *parser, char *line, size_t pos)
{
    line[pos] = '\0';
    char *target = expanser(parser->makefile, remove_blanc(line));
    if (!target)
    {
        parser_error(parser, "unterminated variable reference");
    }
    if (target[0] == '\0')
    {
        free(target);
        parser->current_rule = NULL;
        parser->ignore_recipe = 1;
        return;
    }
    char *deps = expanser(parser->makefile, line + pos + 1);
    if (!deps)
    {
        parser_error(parser, "unterminated variable reference");
    }
    struct rule *rule = add_rule(parser->makefile, target);
    parse_dependencies(rule, deps);
    free(target);
    free(deps);
    parser->current_rule = rule;
    parser->ignore_recipe = 0;
}

static void parse_variable(struct parser *parser, char *line, size_t pos)
{
    line[pos] = '\0';
    char *name = expanser(parser->makefile, remove_blanc(line));
    if (!name)
    {
        parser_error(parser, "unterminated variable reference");
    }
    if (name[0] == '\0')
    {
        parser_error(parser, "empty variable name");
    }
    add_variable(parser->makefile, name, skip_blanks(line + pos + 1));
    free(name);
    parser->current_rule = NULL;
    parser->ignore_recipe = 0;
}

static void parse_line(struct parser *parser, char *line)
{
    remove_newline(line);
    if (is_blank_line(line))
    {
        return;
    }
    if (line[0] == '\t')
    {
        parse_command(parser, line);
        return;
    }
    cut_comment(line);
    if (is_blank_line(line))
    {
        return;
    }
    size_t pos = 0;
    enum line_type type = search_separ(line, &pos);
    if (type == LINE_RULE)
    {
        parse_rule(parser, line, pos);
    }
    else if (type == LINE_VARIABLE)
    {
        parse_variable(parser, line, pos);
    }
    else
    {
        parser_error(parser, "missing separator");
    }
}

void parse_makefile(struct makefile *makefile, char *filename)
{
    assert(makefile != NULL);
    assert(filename != NULL);
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        err(2, "%s", filename);
    }
    struct parser parser = { .makefile = makefile,
                             .current_rule = NULL,
                             .ignore_recipe = 0,
                             .line_number = 0,
                             .filename = filename };
    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, file) != -1)
    {
        parser.line_number++;
        parse_line(&parser, line);
    }
    free(line);
    if (ferror(file))
    {
        err(2, "getline on %s", filename);
    }
    if (fclose(file) == EOF)
    {
        err(2, "fclose on %s", filename);
    }
}
