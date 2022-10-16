CC       = gcc
RECFLAGS = -O3 -Wall -Wextra -Wpedantic 
#-Werror

smallshmake:
	$(CC) -std=c99 -g -o smallsh smallsh.c $(RECFLAGS)
