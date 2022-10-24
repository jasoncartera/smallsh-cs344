CC       = gcc
RECFLAGS = -O3 -Wextra -Wpedantic 
#-Werror

smallshmake:
	$(CC) -std=gnu99 -o smallsh smallsh.c smallsh_lib.c linkedlist.c $(RECFLAGS)
