CC       = gcc
CFLAGS   = -std=c99
RECFLAGS = -O3 -Wall -Wextra -Wpedantic 
#-Werror

smallshmake:
	$(CC) $(CFLAGS) -g -o smallsh smallsh.c $(RECFLAGS)
