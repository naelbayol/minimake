NAME = value
DEP = beta

target: alpha $(DEP)
	echo $(NAME)
