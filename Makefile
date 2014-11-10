all: t2b h2b b2t dec sim

t2b: ops/conv_t2b.c
	gcc -Wall -o t2b ops/conv_t2b.c
h2b: ops/conv_h2b.c
	gcc -Wall -o h2b ops/conv_h2b.c
b2t: ops/conv_b2t.c
	gcc -Wall -o b2t ops/conv_b2t.c
dec: ops/conv_dec.c
	gcc -Wall -o dec ops/conv_dec.c


addu:	ops/addu.c
	gcc -Wall -o addu ops/addu.c
addiu:	ops/addiu.c
	gcc -Wall -o addiu ops/addiu.c
subu:	ops/subu.c
	gcc -Wall -o subu ops/subu.c
multu:	ops/multu.c
	gcc -Wall -o multu ops/multu.c
sll:	ops/sll.c
	gcc -Wall -o sll ops/sll.c
ssum:	ops/sum.c
	gcc -Wall -o ssum ops/sum.c



sim:	simulator.c fpu/C/fpu.h fpu/C/float.o fpu/C/fadd.o fpu/C/fmul.o fpu/C/finv.o fpu/C/itof.o fpu/C/ftoi.o fpu/C/fcmp.o
	gcc -Wall -g -Wextra -O0 -o sim simulator.c fpu/C/float.o fpu/C/fadd.o fpu/C/fmul.o fpu/C/finv.o fpu/C/itof.o fpu/C/ftoi.o fpu/C/fcmp.o -lm

clean:
	rm -f t2b h2b b2t addu subu multu dec addiu sll ssum sim *.o

