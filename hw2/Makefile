FLAGS      = -std=c++11
CPP_DEFS   = -D=HAVE_CONFIG_H
CPP_OPTS   = -Wall -O2
CPPS       = env.c
HDRS       = env.h

all: env

env: $(CPPS) $(HDRS)
	g++ $(FLAGS) $(CPP_OPTS) $(CPP_DEFS) $(CPPS) $(HDRS) -o env

clean:
	rm -f env