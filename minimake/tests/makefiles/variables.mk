SIMPLE = hello
A = one
SPACES   =    a   b
NAME = dynamic
$(NAME)_var = name expanded
DEPS = dep_one dep_two

syntaxes:
	@echo $(SIMPLE) ${SIMPLE} $A

spaces:
	@echo "[$(SPACES)]"

undefined:
	@echo "[$(UNDEFINED_VARIABLE)]"

defined_later:
	@echo $(LATER)

name_expansion:
	@echo $(dynamic_var)

with_deps: $(DEPS)
	@echo deps done

dep_one:
	@echo dep_one

dep_two:
	@echo dep_two

double_dollar:
	@echo '$$'

LATER = defined later
