#ifndef PRINT_H
#define PRINT_H

void printhelp(void);

/* FPレジスタ内容表示器 */
void printFPRegister(unsigned int* fpreg);
/* レジスタ内容表示器 */
void printRegister(unsigned int* reg);
/* 命令数カウント表示器 */
void printOpsCount(unsigned long long opNum[], unsigned long long fpuNum[], unsigned long long breakCount);

/* メモリへの不正アドレスアクセス */
void printErrorAccessToIncorrectAddr(unsigned int pc, unsigned int instruction, unsigned int *memInit, unsigned int address, unsigned long long breakCount);

#endif
