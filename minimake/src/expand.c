#include "expand.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

struct buffer
{
    char *data;
    size_t len;
    size_t capacity;
};

static void init_buffer(struct buffer *buf)
{
    buf->len = 0;
    buf->capacity = 16;
    buf->data = malloc(buf->capacity * sizeof(char));
    check_allocation(buf->data, "malloc for buffer on expanser.");
    buf->data[0] = '\0';
}

static void add_char(struct buffer *buf, char c)
{
    if (buf->len + 1 >= buf->capacity)
    {
        size_t new_capacity = buf->capacity * 2;
        char *new_data = realloc(buf->data, new_capacity * sizeof(char));
        check_allocation(new_data, "realloc for buffer on expanser.");
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    buf->data[buf->len] = c;
    buf->len++;
    buf->data[buf->len] = '\0';
}

static void add_string(struct buffer *buf, char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        add_char(buf, str[i]);
    }
}

static void add_variable_value(struct makefile *makefile, struct buffer *buf,
                               char *name, size_t name_len)
{
    char *var_name = strndup(name, name_len);
    check_allocation(var_name, "strndup for var_name on expanser.");
    struct variable *var = find_variable(makefile, var_name);
    if (var)
    {
        add_string(buf, var->value);
    }
    free(var_name);
}

static size_t parenthesis(struct makefile *makefile, struct buffer *buf,
                          char *chaine)
{
    char closing = ')';
    if (chaine[1] == '{')
    {
        closing = '}';
    }
    char *end = strchr(chaine + 2, closing);
    if (!end)
    {
        return 0;
    }
    size_t name_len = end - (chaine + 2);
    add_variable_value(makefile, buf, chaine + 2, name_len);
    return name_len + 3;
}

static size_t dollar(struct makefile *makefile, struct buffer *buf,
                     char *chaine)
{
    if (chaine[1] == '$')
    {
        add_char(buf, '$');
        return 2;
    }
    if (chaine[1] == '(' || chaine[1] == '{')
    {
        return parenthesis(makefile, buf, chaine);
    }
    if (chaine[1] == '\0')
    {
        add_char(buf, '$');
        return 1;
    }
    add_variable_value(makefile, buf, chaine + 1, 1);
    return 2;
}

char *expanser(struct makefile *makefile, char *chaine)
{
    assert(makefile != NULL);
    assert(chaine != NULL);
    struct buffer buf;
    init_buffer(&buf);
    size_t letter_chaine = 0;
    while (chaine[letter_chaine] != '\0')
    {
        if (chaine[letter_chaine] == '$')
        {
            size_t consumed = dollar(makefile, &buf, chaine + letter_chaine);
            if (consumed == 0)
            {
                free(buf.data);
                return NULL;
            }
            letter_chaine += consumed;
        }
        else
        {
            add_char(&buf, chaine[letter_chaine]);
            letter_chaine++;
        }
    }
    return buf.data;
}
