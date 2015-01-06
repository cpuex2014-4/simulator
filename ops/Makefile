TARGETS = t2b h2b b2t dec

all: $(TARGETS)

t2b: conv_t2b.c
	gcc -Wall -o ../t2b conv_t2b.c
h2b: conv_h2b.c
	gcc -Wall -o h2b conv_h2b.c
b2t: conv_b2t.c
	gcc -Wall -o b2t conv_b2t.c
dec: conv_dec.c
	gcc -Wall -o dec conv_dec.c

nor: nor.c
	gcc -Wall -o dec conv_dec.c




clean:
	rm -f $(TARGETS) *~
