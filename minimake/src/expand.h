#ifndef EXPAND_H
#define EXPAND_H

#include "makefile.h"

// Returns a new string, or NULL on an unterminated variable reference.
char *expanser(struct makefile *makefile, char *chaine);

#endif /* ! EXPAND_H */
