#ifndef ALU_H
#define ALU_H

unsigned int signExt(unsigned int argument);
unsigned int fpuHide(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, unsigned int* fpuNum, unsigned int* labelRec);
unsigned int fpu(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, int* flag, unsigned int* fpuNum, unsigned int* labelRec);

unsigned int sll(unsigned int rs, unsigned int shamt);
unsigned int srl(unsigned int rs, unsigned int shamt);
unsigned int slt(unsigned int rs, unsigned int rt);
unsigned int mult(unsigned int rs, unsigned int rt);
unsigned int or(unsigned int rs, unsigned int rt);
unsigned int ori(unsigned int rs, unsigned int im);
unsigned int and(unsigned int rs, unsigned int rt);
unsigned int subu (unsigned int rs, unsigned int rt);
unsigned int addiu(unsigned int rs, unsigned int Imm, unsigned int stackPointer);
unsigned int addu(unsigned int adduA, unsigned int adduB);

#endif
