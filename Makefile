CFLAGS = -Wall -Werror -lm -g
main: src/ownshell.c
	gcc src/$@.c -o bin/$@ $(CFLAGS)
