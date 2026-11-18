#include "string_list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void init_string_list(struct string_list *list)
{
    assert(list != NULL);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

struct string_list *add_to_string_list(struct string_list *list, char *item)
{
    assert(list != NULL);
    assert(item != NULL);
    if (list->count == list->capacity)
    {
        size_t new_capacity;
        if (list->capacity == 0)
        {
            new_capacity = 1;
        }
        else
        {
            new_capacity = list->capacity * 2;
        }
        char **new_items = realloc(list->items, new_capacity * sizeof(char *));
        check_allocation(new_items, "realloc on add_to_string_list.");
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count] = strdup(item);
    check_allocation(list->items[list->count],
                     "strdup for item on add_to_string_list.");
    list->count++;
    return list;
}

void free_string_list(struct string_list *list)
{
    if (list)
    {
        if (list->count > 0 && list->items)
        {
            for (size_t i = 0; i < list->count; i++)
            {
                free(list->items[i]);
            }
            free(list->items);
            list->items = NULL;
            list->count = 0;
            list->capacity = 0;
        }
    }
}
