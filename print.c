#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "const.h"
#include "print.h"


void printhelp(void) {
	printf("使用法: sim [実行コードファイル] [オプション1] [オプション2]...\n");
	printf("\t--help\t\tヘルプを表示する\n");	// 0
	printf("\t--hide\t\t各命令実行時の詳細データを表示しない\n");	// 1
	printf("\t--break <num>\t\tブレイクポイントを指定\n");		// 2
	printf("\t--reg <num>\t\tレジスタ表示を有効化\n");			// 3
//	printf("\t--serialin <ファイル名>\t\tシリアルポートからの入力を指定\n");	// 4
//	printf("\t--sequential\t\t逐次実行\n");		// 5
	printf("\t--memory <num1> <num2>\t\t<num1>番地から<num2>番地までのメモリ内容を最後に表示\n");		// 6
//	printf("\t--native\t\tFPUをx86ネイティブで実行する\n");		// 7
//	printf("\t--serialout <ファイル名>\t\tシリアルポートからの出力先を指定\n");	// 4

}


/* FPレジスタ内容表示器 */
void printFPRegister(unsigned int* fpreg) {
// unsigned int rZ, rV, rN, rCarry;	// condition register
	int i;

//	printf("R[Condition] ZVNC = %X/%X/%X/%X \n", rZ, rV, rN, rCarry);
	for(i=0; i<FPREGSIZE; i++) {
		if(i%8 == 0) {
			printf("FP[%2d->%2d] : %8X ", i, i+7, fpreg[i]);
		} else if( (i+1)%8 == 0 ) {
			printf("%8X(",fpreg[i]);
//			printFloat(fpreg[i]);
			printf(")\n");
		} else {
			printf("%8X ",fpreg[i]);
		}
	}

/*
	for(i=0; i<FPREGSIZE; i++) {
		if(i == 0) {
			printf("$zr= 0x%-6X ",reg[i]);
		} else if( i == 1 ) {
			printf("$at= 0x%-6X ",reg[i]);
		} else if( i == 2 || i == 3 ) {
			printf("$v%d= 0x%-6X ", i-2,reg[i]);
		} else if( i >= 4 && i < 8 ) {
			printf("$a%d= 0x%-6X ", i-4,reg[i]);
		} else if( i >= 8 && i < 16 ) {
			printf("$t%d= 0x%-6X ", i-8, reg[i]);
			if(i==10) printf("\n");
		} else if( i >= 16 && i < 24 ) {
			printf("$s%d= 0x%-6X ", i-16, reg[i]);
			if(i==21) printf("\n");
		} else if( i == 24 || i == 25 ) {
			printf("$t%d= 0x%-6X ", i-16, reg[i]);
		} else if( i == 26 || i == 27 ) {
			printf("$k%d= 0x%-6X ",i-26,reg[i]);
		} else if( i == 28 ) {
			printf("$gp= 0x%-6X ",reg[i]);
		} else if( i == 29 ) {
			printf("$sp= 0x%-6X ",reg[i]);
		} else if( i == 30 ) {
			printf("$fp= 0x%-6X ",reg[i]);
		} else if( i == 31 ) {
			printf("$ra= 0x%-6X",reg[i]);
		} else {
			printf("N/A");
		}
	}
*/
	printf("\n");
}


/* レジスタ内容表示器 */
void printRegister(unsigned int* reg) {
// unsigned int rZ, rV, rN, rCarry;	// condition register
	int i;

//	printf("R[Condition] ZVNC = %X/%X/%X/%X \n", rZ, rV, rN, rCarry);
/*	for(i=0; i<REGSIZE; i++) {
		if(i%16 == 0) 
			printf("R[%2d->%2d] : ", i, i+16);
		else if( (i+1)%16 == 0 ) {
			printf("%4X\n",reg[i]);
		} else {
			printf("%4X ",reg[i]);
		}
	}
*/

	for(i=0; i<REGSIZE; i++) {
		if(i==0) printf("\t");
		if(reg[i] == 0) continue;
		if(i == 0) {
			printf("$zr=0x%-6X ",reg[i]);
		} else if( i == 1 ) {
			printf("$at=0x%-6X ",reg[i]);
		} else if( i == 2 || i == 3 ) {
			printf("$v%d=0x%-6X ", i-2,reg[i]);
		} else if( i >= 4 && i < 8 ) {
			printf("$a%d=0x%-6X ", i-4,reg[i]);
		} else if( i >= 8 && i < 16 ) {
			printf("$t%d=0x%-6X ", i-8, reg[i]);
			if(i==10) printf("\n");
		} else if( i >= 16 && i < 24 ) {
			printf("$s%d=0x%-6X ", i-16, reg[i]);
			if(i==21) printf("\n");
		} else if( i == 24 || i == 25 ) {
			printf("$t%d=0x%-6X ", i-16, reg[i]);
		} else if( i == 26 || i == 27 ) {
			printf("$k%d=0x%-6X ",i-26,reg[i]);
		} else if( i == 28 ) {
			printf("$gp=0x%-6X ",reg[i]);
		} else if( i == 29 ) {
			printf("$sp=0x%-6X ",reg[i]);
		} else if( i == 30 ) {
			printf("$fp=0x%-6X ",reg[i]);
		} else if( i == 31 ) {
			printf("$ra=0x%-6X",reg[i]);
		} else {
			printf("N/A");
		}
	}
	printf("\n");
}


