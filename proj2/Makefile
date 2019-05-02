CC=gcc
FLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic -g

.PHONY=clean all

all: proj2

proj2: proj2.c
	$(CC) $(FLAGS) $^ -lpthread -lrt -o proj2
clean:
	rm -f proj2
