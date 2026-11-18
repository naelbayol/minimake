all: first second
	@echo all done

first:
	@echo first

second: first
	@echo second

empty:

nothing: empty

missing_dep: does_not_exist
	@echo never printed

output: input
	touch output
