CC       = gcc
RECFLAGS = -O3 -Wextra -Wpedantic 
#-Werror

smallshmake:
	$(CC) -g -o smallsh smallsh.c smallsh_lib.c $(RECFLAGS)
