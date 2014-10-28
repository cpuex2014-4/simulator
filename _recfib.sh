#! /bin/sh

make
t2b fib_recursive.b frec.b
sim frec.b --break 100 --serial serial.in > resultrecfib.txt
t2b test.b test.bin
#sim test.bin --break 20

