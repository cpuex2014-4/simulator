all: t2b h2b b2t sim

t2b: conv_t2b.c
	gcc -Wall -o t2b conv_t2b.c
h2b: conv_h2b.c
	gcc -Wall -o h2b conv_h2b.c

b2t: conv_b2t.c
	gcc -Wall -o b2t conv_b2t.c

sim:	simulator.c
	gcc -Wall -o sim simulator.c






