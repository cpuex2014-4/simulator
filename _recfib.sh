#! /bin/sh

make
t2b fib_recursive.b frec.b
sim frec.b --break 500 > resultrecfib.txt

