.PHONY: all clean

CC = gcc
FLAGS = -ggdb3 -g -lm -fno-stack-protector

SRCS = ${wildcard *.c}
HEADERS = ${wildcard *.h}

all: main

main: ${SRCS} ${HEADERS}
	@echo "Compiling"
	${CC} ${FLAGS} -o $@ $^

valgrind:
	valgrind --log-file=log \
			--tool=memcheck \
			--track-origins=yes \
			--show-reachable=yes \
			--show-leak-kinds=all \
			--leak-check=full \
			--verbose \
			./main

clean:
	@echo "Cleaning"
	rm -rvf main
