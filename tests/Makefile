FLAGS = -std=c99 -Wall -O1

all: tests

tests: t3.c
	gcc -pthread ${FLAGS} -o t3 t3.c

clean:
	rm -f t3