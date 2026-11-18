CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -MMD -MP

DEBUG_FLAGS =
CFLAGS += $(DEBUG_FLAGS)
LDFLAGS += $(DEBUG_FLAGS)

SRC = src/build.c src/error.c src/exec.c src/expand.c src/main.c \
      src/makefile.c src/option.c src/parsing.c src/print.c src/string_list.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

minimake: $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

-include $(DEP)

check: minimake
	sh tests/testsuite.sh

clean:
	$(RM) $(OBJ) $(DEP) minimake

.PHONY: check clean
