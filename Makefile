# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Targets
all: shell

shell: main.c
	clear
	$(CC) $(CFLAGS) -o $@ $^ -lreadline 

clean:
	rm -f shell
