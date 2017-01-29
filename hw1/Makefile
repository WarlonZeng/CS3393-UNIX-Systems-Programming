FLAGS      = -std=c++11
CPP_DEFS   = -D=HAVE_CONFIG_H
CPP_OPTS   = -Wall -O2

default: all

all: myLife.c
	g++ $(FLAGS) ${CPP_OPTS} ${CPP_DEFS} -o myLife myLife.c

clean:
	rm -f all