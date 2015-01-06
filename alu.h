#ifndef ALU_H
#define ALU_H

unsigned int signExt(unsigned int argument);
unsigned int sw(unsigned int rt, unsigned int address, unsigned int* memory);
unsigned int lw(unsigned int address, unsigned int* memory);

unsigned int fpuHide(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, unsigned long long* fpuNum, unsigned int* labelRec);
unsigned int fpu(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, int* flag, unsigned long long* fpuNum, unsigned int* labelRec);

unsigned int sll(unsigned int rs, unsigned int shamt);
unsigned int srl(unsigned int rs, unsigned int shamt);
unsigned int sra(unsigned int rs, unsigned int shamt);
unsigned int slt(unsigned int rs, unsigned int rt);
unsigned int slti(unsigned int rs, unsigned int im);
unsigned int sltiu(unsigned int rs, unsigned int im);
unsigned int or(unsigned int rs, unsigned int rt);
unsigned int nor(unsigned int rs, unsigned int rt);
unsigned int xor(unsigned int rs, unsigned int rt);
unsigned int ori(unsigned int rs, unsigned int im);
unsigned int and(unsigned int rs, unsigned int rt);
unsigned int subu (unsigned int rs, unsigned int rt);
unsigned int addiu(unsigned int rs, unsigned int Imm);
unsigned int addu(unsigned int adduA, unsigned int adduB);

#endif
