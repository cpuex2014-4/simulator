CC = gcc
CFLAGS = -lm -Wall -Wextra -g
TARGET = sim
SIM = simulator.o print.o alu.o
FPU = fpu/C/float.o fpu/C/fadd.o fpu/C/fmul.o fpu/C/finv.o fpu/C/itof.o fpu/C/ftoi.o fpu/C/fcmp.o fpu/C/fdiv.o

all: $(TARGET)

$(TARGET): $(FPU) $(SIM)
	$(CC) -o $(TARGET) $(SIM) $(FPU) $(CFLAGS)

clean:
	rm -f $(SIM) $(FPU) $(TARGET) *.o *~
