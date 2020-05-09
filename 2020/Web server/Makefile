.PHONY: all clean

CC = gcc
FLAGS = -ggdb3 -g -lm -fno-stack-protector

SRCS = ${wildcard *.c}
HEADERS = ${wildcard *.h}

all: main

main: ${SRCS} ${HEADERS}
	@echo "Compiling"
	${CC} ${FLAGS} -o $@ $^

clean:
	@echo "Cleaning"
	rm -rvf main
