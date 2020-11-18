ownshell: src/ownshell.c
	gcc src/$@.c -o src/$@ -Wall -Werror -lm -g
