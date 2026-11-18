#ifndef ERROR_H
#define ERROR_H

// Exits with status 2 when to_check is NULL.
void check_allocation(void *to_check, char *message);
void allocation_error(char *message);
void error_message(char *message);

#endif /* ! ERROR_H */
