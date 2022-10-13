CC       = gcc
CFLAGS   = -std=c99
RECFLAGS = -O3 -Wall -Wextra -Wpedantic 
#-Werror

smallshmake:
	$(CC) $(CFLAGS) -o smallsh smallsh.c $(RECFLAGS)
