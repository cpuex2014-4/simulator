all: t2b h2b b2t sim addu addui subu multu

t2b: conv_t2b.c
	gcc -Wall -o t2b conv_t2b.c
h2b: conv_h2b.c
	gcc -Wall -o h2b conv_h2b.c

b2t: conv_b2t.c
	gcc -Wall -o b2t conv_b2t.c

sim:	simulator.c
	gcc -O3 -Wall -o sim simulator.c

addu:	addu.c
	gcc -Wall -o addu addu.c
addui:	addui.c
	gcc -Wall -o addui addui.c
subu:	subu.c
	gcc -Wall -o subu subu.c
multu:	multu.c
	gcc -Wall -o multu multu.c





