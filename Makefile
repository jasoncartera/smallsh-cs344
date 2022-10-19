CC       = gcc
RECFLAGS = -O3 -Wall -Wextra -Wpedantic 
#-Werror

smallshmake:
	$(CC) -g -o smallsh smallsh_lib.c smallsh.c $(RECFLAGS)
