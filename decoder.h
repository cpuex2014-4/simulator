#ifndef DEC_H
#define DEC_H

unsigned int decoder (unsigned int pc, unsigned int instruction, unsigned int* memory, unsigned int* memInit,unsigned char* input, unsigned char* srOut, unsigned long long breakCount, int* flag, unsigned int* reg, unsigned int* opNum, unsigned int* labelRec, FILE* soFile, unsigned int opcode);

#endif
