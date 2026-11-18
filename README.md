# Minimake

An academic reimplementation of a subset of `make` in C. Minimake reads a makefile, expands variables, resolves dependencies recursively and runs only the recipes needed to bring the requested targets up to date.

This project explores line-oriented parsing, string handling, dependency graphs, file timestamps and process management with `fork`, `execv` and `waitpid`.

## Features

- Makefile lookup: `makefile`, then `Makefile`, or an explicit file given with `-f`.
- Variable definitions with `NAME = value`. Redefining a variable replaces its previous value.
- Variable references with `$(NAME)`, `${NAME}` and single-character `$X`, plus `$$` for a literal `$`. Undefined variables expand to an empty string.
- Expansion in variable names, target names, dependency lists and recipes.
- Rules with any number of dependencies and tab-indented recipe lines. `#` starts a comment on rule and variable lines.
- Recursive dependency resolution. Each rule is processed at most once per run.
- Rebuild decisions based on file existence and modification times, with nanosecond precision when the file system provides it. A target is also rebuilt when one of its dependencies was rebuilt during the same run.
- Recipe lines echoed before execution, unless prefixed with `@`. Each line runs in its own `/bin/sh -c` process, and the first failing command stops the build.
- GNU make style diagnostics: `Nothing to be done for 'target'.`, `'target' is up to date.`, `No rule to make target ...`, `missing separator`, `recipe commences before first target`.
- A `-p` mode that prints the parsed variables and rules without running anything.
- Exit status `2` on errors, like GNU make.

## How it works

```text
makefile ──> parser ──> variables + rules ──> build (depth-first)
                                                     │
                                                     ▼
                    waitpid <── execv("/bin/sh -c") <── fork
```

### Parsing

The makefile is read line by line with `getline`.

1. A line starting with a tab is a recipe line for the current rule.
2. Any other line has its comment removed and is classified by its first separator: `:` for a rule, `=` for a variable. A line with neither is a `missing separator` error.
3. Target names and dependency lists are expanded when the rule is read. Recipes are stored as written and expanded just before execution, so a variable defined after a rule is still visible in its recipe.

### Building

For each requested target (or the first rule when none is given), minimake walks the dependency graph depth-first:

1. Each dependency is built first if a rule exists for it. Otherwise, it must exist as a file.
2. The target is rebuilt when its file does not exist and the rule has commands, when a dependency was rebuilt in this run, or when a dependency file is newer than the target.
3. The rule is marked as done, so a dependency shared by several targets is handled only once.

### Execution

Each recipe line is expanded, stripped of its leading blanks and `@` prefix, printed unless silent, then executed by a child process running `/bin/sh -c`. The parent waits with `waitpid`. A non-zero exit status stops the build with `*** [target] Error N`.

## Requirements

- Linux or another POSIX system: the code uses `fork`, `execv`, `waitpid`, `stat` and `getline`.
- GCC and GNU Make to build the project.
- A POSIX shell for the test script.

## Build

From the repository root:

```sh
cd minimake
make
```

This generates the `minimake` executable.

Additional targets:

```sh
make check   # Build minimake and run the test script
make clean   # Remove objects, dependency files and the executable
```

The Makefile compiles in C99 with strict warnings (`-pedantic -Werror -Wall -Wextra -Wvla`) and generates header dependencies automatically.

## Usage

```text
./minimake [options] [target] ...
```

| Option | Behavior |
|---|---|
| `-f FILE`, `--file=FILE`, `--makefile=FILE` | Read `FILE` as the makefile. |
| `-p`, `--print-data-base`, `--pretty-print` | Print the parsed variables and rules without running them. |
| `-h`, `--help` | Print the help message and exit. |

Without a target, minimake builds the first rule of the makefile. Several targets are built in the order given.

### Example

```make
CC = gcc
CFLAGS = -Wall -Wextra

prog: main.o
	$(CC) -o prog main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c
```

```console
$ ./minimake prog
gcc -Wall -Wextra -c main.c
gcc -o prog main.o
$ ./minimake prog
minimake: 'prog' is up to date.
$ touch main.c
$ ./minimake prog
gcc -Wall -Wextra -c main.c
gcc -o prog main.o
```

### Printing a makefile

```make
NAME = value
DEP = beta

target: alpha $(DEP)
	echo $(NAME)
```

```console
$ ./minimake -p
# variables
'NAME' = 'value'
'DEP' = 'beta'
# rules
(target): [alpha] [beta]
    'echo $(NAME)'
```

Dependencies appear expanded, while recipes are shown as written.

## Tests

```sh
make check
```

The shell test suite runs minimake on small makefiles from `tests/makefiles/` and compares the output and exit status with the expected values. It covers:

- Command-line options and argument errors.
- The `-p` output and its parsing edge cases.
- Syntax errors such as a missing separator or a recipe before the first target.
- The three variable syntaxes, spacing around `=`, undefined variables and `$$`.
- Target selection, default target and repeated targets.
- Rebuild decisions based on timestamps, `up to date` and `Nothing to be done` messages.
- Command echoing, silent commands, separate shell processes and failing commands.
- A real compilation with `gcc`, including a rebuild after a source change and a compilation error.

The script prints `PASS` or `FAIL` for each test, then a summary, and returns a non-zero status if a test fails.

## Project structure

```text
.
├── README.md
└── minimake/
    ├── Makefile
    ├── src/
    │   ├── main.c          # Entry point and makefile selection
    │   ├── option.c        # Command-line options and help message
    │   ├── parsing.c       # Line-by-line makefile parser
    │   ├── makefile.c      # Storage for variables and rules
    │   ├── expand.c        # Variable expansion
    │   ├── build.c         # Dependency resolution and rebuild decisions
    │   ├── exec.c          # Recipe execution with fork, execv and waitpid
    │   ├── print.c         # Output of the -p option
    │   ├── string_list.c   # Dynamic arrays of strings
    │   └── error.c         # Error helpers
    └── tests/
        ├── testsuite.sh    # Test script executed by make check
        └── makefiles/      # Makefiles used by the tests
```

## Limitations

- **Subset of make:** pattern and implicit rules, automatic variables such as `$@` or `$<`, the `:=`, `+=` and `?=` assignments, functions, conditionals, `include` and parallel jobs are not supported.
- **One target per rule:** `a b: c` defines a single target named `a b`.
- **No line continuation:** a trailing `\` does not join lines.
- **Single expansion pass:** a variable value containing another reference, such as `A = $(B)`, is inserted as written instead of being expanded again.
- **No cycle detection:** circular dependencies are not reported and lead to unbounded recursion.

This is an educational project focused on the core of `make`: parsing, dependency resolution, timestamps and process execution.
