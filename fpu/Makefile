CC   = gcc
CFLAGS = -lm -Wall
TARGET1 = fpuTest
TARGET2 = testGen
OBJS1  = float.o fadd.o fmul.o finv.o itof.o ftoi.o fcmp.o fpuTest.o
OBJS2  = float.o testGen.o

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1)
	$(CC) -o $(TARGET1) $(OBJS1) $(CFLAGS)

$(TARGET2): $(OBJS2)
	$(CC) -o $(TARGET2) $(OBJS2) $(CFLAGS)

.c.o:
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f $(TARGET1) $(TARGET2) *~ *.o
