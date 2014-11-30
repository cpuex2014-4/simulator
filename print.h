#ifndef PRINT_H
#define PRINT_H

void printhelp(void);

/* FPレジスタ内容表示器 */
void printFPRegister(unsigned int* fpreg);
/* レジスタ内容表示器 */
void printRegister(unsigned int* reg);
/* 命令数カウント表示器 */
void printOpsCount(unsigned int opNum[], unsigned int fpuNum[], unsigned long long breakCount);

#endif
