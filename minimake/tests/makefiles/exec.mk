logged:
	echo logged

silent:
	@echo silent

leading_blanks:
	    echo trimmed

blank_lines:
	echo before

	echo after

false_stops:
	false
	echo never printed

shell_var:
	X=42
	echo "[$$X]"

unterminated:
	@echo $(X
