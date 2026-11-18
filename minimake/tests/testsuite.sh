#!/bin/sh

root="$(cd "$(dirname "$0")/.." && pwd)"
bin="$root/minimake"
mk="$root/tests/makefiles"
tmp="$(mktemp -d)"
work="$tmp/work"
trap 'rm -rf "$tmp"' EXIT

total=0
passed=0

help='Usage: minimake [options] [target] ...
Options:
  -f FILE, --file=FILE, --makefile=FILE
      Read FILE as a makefile.
  -h, --help
      Print this message and exit.
  -p, --print-data-base, --pretty-print
      Print the parsed makefile without running it.
'
printed="# variables\n'NAME' = 'value'\n'DEP' = 'beta'\n# rules\n"
printed="$printed(target): [alpha] [beta]\n    'echo \$(NAME)'\n"
build="gcc -Wall -Wextra -c main.c\ngcc -o prog main.o\n"

new_work()
{
    rm -rf "$work"
    mkdir "$work"
}

run_test()
{
    name="$1"
    code="$2"
    printf '%b' "$3" > "$tmp/expected"
    shift 3
    (cd "$work" && "$bin" "$@") > "$tmp/out" 2> /dev/null
    if [ "$?" -eq "$code" ] && cmp -s "$tmp/expected" "$tmp/out"; then
        echo "PASS $name"
        passed=$((passed + 1))
    else
        echo "FAIL $name"
    fi
    total=$((total + 1))
}

new_work

run_test option_help_priority 0 "$help" all -p -h
run_test option_empty_argument 2 "" "" -h
run_test option_unknown 2 "" --unknown
run_test option_file_not_found 2 "" -f does_not_exist

run_test print_syntax_test 0 "$(cat "$root/tests/syntax-test.out")\n" \
    -p -f "$root/tests/Makefile.syntax-test"
run_test print_variables_and_rules 0 "$printed" -p -f "$mk/print.mk"
run_test print_last_file_wins 0 "$printed" -p -f "$mk/build.mk" -f "$mk/print.mk"

run_test parse_missing_separator 2 "" -f "$mk/missing_separator.mk"
run_test parse_recipe_before_rule 2 "" -f "$mk/recipe_before_rule.mk"
run_test parse_blank_line_in_recipe 0 "echo before\nbefore\necho after\nafter\n" \
    -f "$mk/exec.mk" blank_lines

run_test var_three_syntaxes 0 "hello hello one\n" -f "$mk/variables.mk" syntaxes
run_test var_spaces_around_equal 0 "[a   b]\n" -f "$mk/variables.mk" spaces
run_test var_undefined_is_empty 0 "[]\n" -f "$mk/variables.mk" undefined

run_test expand_defined_after_rule 0 "defined later\n" \
    -f "$mk/variables.mk" defined_later
run_test expand_variable_name 0 "name expanded\n" \
    -f "$mk/variables.mk" name_expansion
run_test expand_in_dependencies 0 "dep_one\ndep_two\ndeps done\n" \
    -f "$mk/variables.mk" with_deps
run_test expand_double_dollar 0 "\$\n" -f "$mk/variables.mk" double_dollar

run_test select_default_first_rule 0 "first\nsecond\nall done\n" \
    -f "$mk/build.mk"
run_test select_targets_in_order 0 \
    "first\nsecond\nminimake: 'first' is up to date.\n" \
    -f "$mk/build.mk" second first
run_test select_same_target_twice 0 \
    "first\nsecond\nall done\nminimake: 'all' is up to date.\n" \
    -f "$mk/build.mk" all all
run_test select_no_rule 2 "" -f "$mk/build.mk" unknown

run_test build_no_rule_needed_by 2 "" -f "$mk/build.mk" missing_dep
run_test build_nothing_to_be_done 0 \
    "minimake: Nothing to be done for 'nothing'.\n" -f "$mk/build.mk" nothing
touch -t 202001010000 "$work/input"
touch "$work/output"
run_test build_up_to_date 0 "minimake: 'output' is up to date.\n" \
    -f "$mk/build.mk" output
touch -t 202001010000 "$work/output"
touch -t 202101010000 "$work/input"
run_test build_newer_dependency 0 "touch output\n" -f "$mk/build.mk" output

run_test exec_false_stops 2 "false\n" -f "$mk/exec.mk" false_stops
run_test exec_separate_processes 0 "X=42\necho \"[\$X]\"\n[]\n" \
    -f "$mk/exec.mk" shell_var
run_test exec_unterminated_variable 2 "" -f "$mk/exec.mk" unterminated

run_test log_command 0 "echo logged\nlogged\n" -f "$mk/exec.mk" logged
run_test log_silent 0 "silent\n" -f "$mk/exec.mk" silent
run_test log_leading_blanks_removed 0 "echo trimmed\ntrimmed\n" \
    -f "$mk/exec.mk" leading_blanks

new_work
printf 'int main(void)\n{\n    return 0;\n}\n' > "$work/main.c"
touch -t 201901010000 "$work/main.c"
run_test real_first_build 0 "$build" -f "$mk/program.mk"
run_test real_up_to_date 0 "minimake: 'prog' is up to date.\n" \
    -f "$mk/program.mk"
touch -t 202001010000 "$work/main.o" "$work/prog"
touch -t 202101010000 "$work/main.c"
run_test real_modified_source 0 "$build" -f "$mk/program.mk"
new_work
printf 'int main(void)\n{\n    return\n}\n' > "$work/main.c"
run_test real_compile_error 2 "gcc -Wall -Wextra -c main.c\n" \
    -f "$mk/program.mk"

echo "$passed/$total tests passed"
[ "$passed" -eq "$total" ]
