#ifndef STRING_LIST_H
#define STRING_LIST_H

#include <stddef.h>

struct string_list
{
    char **items;
    size_t count;
    size_t capacity;
};

void init_string_list(struct string_list *list);
// Appends a copy of item: the caller keeps ownership of item.
struct string_list *add_to_string_list(struct string_list *list, char *item);
void free_string_list(struct string_list *list);

#endif /* ! STRING_LIST_H */
