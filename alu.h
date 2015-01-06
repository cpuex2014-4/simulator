#ifndef ALU_H
#define ALU_H

unsigned int signExt(unsigned int argument);
unsigned int sw(unsigned int rt, unsigned int address, unsigned int* memory);
unsigned int lw(unsigned int address, unsigned int* memory);

unsigned int fpuHide(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, unsigned long long* fpuNum, unsigned int* labelRec);
unsigned int fpu(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, int* flag, unsigned long long* fpuNum, unsigned int* labelRec);

unsigned int sll(unsigned int rs, unsigned int shamt);
unsigned int sllv(unsigned int rs, unsigned int rt);
unsigned int srl(unsigned int rs, unsigned int shamt);
unsigned int srlv(unsigned int rs, unsigned int rt);
unsigned int sra(unsigned int rs, unsigned int shamt);
unsigned int srav(unsigned int rs, unsigned int rt);
unsigned int slt(unsigned int rs, unsigned int rt);
unsigned int slti(unsigned int rs, unsigned int im);
unsigned int sltiu(unsigned int rs, unsigned int im);
unsigned int or(unsigned int rs, unsigned int rt);
unsigned int nor(unsigned int rs, unsigned int rt);
unsigned int xor(unsigned int rs, unsigned int rt);
unsigned int xori(unsigned int rs, unsigned int im);
unsigned int ori(unsigned int rs, unsigned int im);
unsigned int and(unsigned int rs, unsigned int rt);
unsigned int andi(unsigned int rs, unsigned int im);
unsigned int subu (unsigned int rs, unsigned int rt);
unsigned int addiu(unsigned int rs, unsigned int im);
unsigned int addu(unsigned int rs, unsigned int rt);

#endif
