#ifndef ALU_H
#define ALU_H

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
