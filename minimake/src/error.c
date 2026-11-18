#include "error.h"

#include <err.h>

void check_allocation(void *to_check, char *message)
{
    if (!to_check)
    {
        allocation_error(message);
    }
}

void allocation_error(char *message)
{
    err(2, "Memory allocation failed: %s", message);
}

void error_message(char *message)
{
    errx(2, "%s", message);
}
