#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "const.h"
#include "print.h"

typedef union {
  struct {
    uint32_t frac: 23;
    uint32_t exp: 8;
    uint32_t sign: 1;
  } Float;
  float f;
  uint32_t u;
} uni;

void printhelp(void) {
	printf("使用法: sim [入力ファイル] [オプション1] <num> [オプション2] <num1> <num2>...\n");
	printf("\t--help\t\t\t\tヘルプを表示する\n");	// 0
	printf("\t--show\t\t\t\t各命令実行時の詳細データを表示する\n");	// 1
	printf("\t%s <num>\t\t\t命令実行回数を指定\n", BREAKPOINT);		// 2
	printf("\t%s <num>\t\t\tレジスタ表示を有効化\n", PRINTREG);			// 3
	printf("\t%s\t\t\t逐次実行 (未実装)\n", SEQUENTIAL);		// 5
	printf("\t--memory <num1> <num2>\t\t<num1>番地から<num2>番地までのメモリ内容を最後にmemory.outへ出力(未指定時:全領域出力)\n");		// 6
//	printf("\t--native\t\t\tFPUをx86ネイティブで実行する (未実装)\n");		// 7
	printf("\t%s <ファイル名>\tシリアルポートからの出力先を指定(未指定時:serial.out)\n", SERIALOUT);	// 8
//	printf("\t--label\t\t\t\tラベルの飛び先カウント (未実装)\n");	// 9
//	printf("\t--fast\t\t\t各命令実行時の詳細データを表示せず、Cネイティブ実装の浮動小数点演算を行う\n");	// 10
}

float printFloat(unsigned int fpreg) {
	uni temp;
	temp.u = fpreg;
	return temp.f;
}

/* FPレジスタ内容表示器 */
void printFPRegister(unsigned int* fpreg) {
// unsigned int rZ, rV, rN, rCarry;	// condition register
	int i=0,j=0;

//	printf("R[Condition] ZVNC = %X/%X/%X/%X \n", rZ, rV, rN, rCarry);
	for(i=0; i<FPREGSIZE; i++) {
		if(i%4 == 0) {
			printf("\tFP[%2d->%2d] : %12X ", i, i+3, fpreg[i]);
		} else if( (i+1)%4 == 0 ) {
			printf("%12X\n",fpreg[i]);
			for(j=i-3;j<i+1;j++) {
				if(j%4 == 0) {
					printf("\t             %12.3f ", printFloat(fpreg[j]));
				} else if( (j+1)%4 == 0 ) {
					printf("%12.3f\n",printFloat(fpreg[j]));
					break;
				} else {
					printf("%12.3f ",printFloat(fpreg[j]));
				}
			}
		} else {
			printf("%12X ",fpreg[i]);
		}


	}

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
			if(i==10) printf("\n\t");
		} else if( i >= 16 && i < 24 ) {
			printf("$s%d=0x%-6X ", i-16, reg[i]);
			if(i==21) printf("\n\t");
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

void printOpsCount(unsigned long long opNum[], unsigned long long fpuNum[], unsigned long long breakCount) {
/* ops & function */

	printf("\n(OP)\t: \t(Num), \t(Ratio)\n");
	if(opNum[128+ADDU] != 0) { printf("ADDU	: %10llu, %05.2f (%%)\n", opNum[128+ADDU], (double) 100*opNum[128+ADDU]/breakCount); }
	if(opNum[ADDIU] != 0) { printf("ADDIU	: %10llu, %05.2f (%%)\n", opNum[ADDIU], (double) 100*opNum[ADDIU]/breakCount); }
	if(opNum[128+SUBU] != 0) { printf("SUBU	: %10llu, %05.2f (%%)\n", opNum[128+SUBU], (double) 100*opNum[128+SUBU]/breakCount); }
	if(opNum[128+AND] != 0) { printf("AND 	: %10llu, %05.2f (%%)\n", opNum[128+AND], (double) 100*opNum[128+AND]/breakCount); }
	if(opNum[ANDI] != 0) { printf("AND 	: %10llu, %05.2f (%%)\n", opNum[ANDI], (double) 100*opNum[ANDI]/breakCount); }
	if(opNum[128+OR] != 0) { printf("OR  	: %10llu, %05.2f (%%)\n", opNum[128+OR], (double) 100*opNum[128+OR]/breakCount); }
	if(opNum[128+SLL] != 0) { printf("SLL 	: %10llu, %05.2f (%%)\n", opNum[128+SLL], (double) 100*opNum[128+SLL]/breakCount); }
	if(opNum[128+SRL] != 0) { printf("SRL 	: %10llu, %05.2f (%%)\n", opNum[128+SRL], (double) 100*opNum[128+SRL]/breakCount); }
	if(opNum[LW] != 0) { printf("LW  	: %10llu, %05.2f (%%)\n", opNum[LW], (double) 100*opNum[LW]/breakCount); }
	if(opNum[SW] != 0) { printf("SW  	: %10llu, %05.2f (%%)\n", opNum[SW], (double) 100*opNum[SW]/breakCount); }
	if(opNum[JUMP] != 0) { printf("JUMP	: %10llu, %05.2f (%%)\n", opNum[JUMP], (double) 100*opNum[JUMP]/breakCount); }
	if(opNum[JAL] != 0) { printf("JAL 	: %10llu, %05.2f (%%)\n", opNum[JAL], (double) 100*opNum[JAL]/breakCount); }
	if(opNum[128+JR] != 0) { printf("JR  	: %10llu, %05.2f (%%)\n", opNum[128+JR], (double) 100*opNum[128+JR]/breakCount); }
	if(opNum[BEQ] != 0) { printf("BEQ 	: %10llu, %05.2f (%%)\n", opNum[BEQ], (double) 100*opNum[BEQ]/breakCount); }
	if(opNum[BNE] != 0) { printf("BNE 	: %10llu, %05.2f (%%)\n", opNum[BNE], (double) 100*opNum[BNE]/breakCount); }
	if(opNum[128+SLT] != 0) { printf("SLT 	: %10llu, %05.2f (%%)\n", opNum[128+SLT], (double) 100*opNum[128+SLT]/breakCount); }
	if(opNum[SRCV] != 0) { printf("RRB 	: %10llu, %05.2f (%%)\n", opNum[SRCV], (double) 100*opNum[SRCV]/breakCount); }
	if(opNum[SSND] != 0) { printf("RSB 	: %10llu, %05.2f (%%)\n", opNum[SSND], (double) 100*opNum[SSND]/breakCount); }
	printf("\n");
//	printf("\n(Func)\t: \t(Num), \t(Ratio)\n");
//	printf("\n");

/* fpu */
	printf("\n(FPU)\t: \t(Num), \t(Ratio)\n");
	if(fpuNum[BC1F] != 0) { printf("BC1F	: %10llu, %05.2f (%%)\n", fpuNum[BC1F], (double) 100*fpuNum[BC1F]/breakCount); }
	if(fpuNum[BC1T] != 0) { printf("BC1T	: %10llu, %05.2f (%%)\n", fpuNum[BC1T], (double) 100*fpuNum[BC1T]/breakCount); }
	if(fpuNum[FMFC] != 0) { printf("FMFC	: %10llu, %05.2f (%%)\n", fpuNum[FMFC], (double) 100*fpuNum[FMFC]/breakCount); }
	if(fpuNum[FMTC] != 0) { printf("FMTC	: %10llu, %05.2f (%%)\n", fpuNum[FMTC], (double) 100*fpuNum[FMTC]/breakCount); }
	if(fpuNum[MOVSF] != 0) { printf("MOVSF	: %10llu, %05.2f (%%)\n", fpuNum[MOVSF], (double) 100*fpuNum[MOVSF]/breakCount); }
	if(fpuNum[FADDS] != 0) { printf("FADD.S	: %10llu, %05.2f (%%)\n", fpuNum[FADDS], (double) 100*fpuNum[FADDS]/breakCount); }
	if(fpuNum[FSUBS] != 0) { printf("FSUB.S	: %10llu, %05.2f (%%)\n", fpuNum[FSUBS], (double) 100*fpuNum[FSUBS]/breakCount); }
	if(fpuNum[FMULS] != 0) { printf("FMUL.S	: %10llu, %05.2f (%%)\n", fpuNum[FMULS], (double) 100*fpuNum[FMULS]/breakCount); }
	if(fpuNum[FDIVS] != 0) { printf("FDIV.S	: %10llu, %05.2f (%%)\n", fpuNum[FDIVS], (double) 100*fpuNum[FDIVS]/breakCount); }
	if(fpuNum[CEQ] != 0) { printf("CEQ 	: %10llu, %05.2f (%%)\n", fpuNum[CEQ], (double) 100*fpuNum[CEQ]/breakCount); }
	if(fpuNum[COLT] != 0) { printf("COLT	: %10llu, %05.2f (%%)\n", fpuNum[COLT], (double) 100*fpuNum[COLT]/breakCount); }
	if(fpuNum[COLE] != 0) { printf("COLE	: %10llu, %05.2f (%%)\n", fpuNum[COLE], (double) 100*fpuNum[COLE]/breakCount); }
	if(fpuNum[FTOI] != 0) { printf("FTOI	: %10llu, %05.2f (%%)\n", fpuNum[FTOIF], (double) 100*fpuNum[FTOIF]/breakCount); }
	if(fpuNum[ITOF] != 0) { printf("ITOF	: %10llu, %05.2f (%%)\n", fpuNum[ITOF], (double) 100*fpuNum[ITOF]/breakCount); }
	if(fpuNum[SQRT] != 0) { printf("FSQRT	: %10llu, %05.2f (%%)\n", fpuNum[SQRT], (double) 100*fpuNum[SQRT]/breakCount); }

}

/* 命令名表示器 */
void printOp(unsigned int opcode) {
	switch(opcode) {
		case(1) :
			;
			break;
		case(2) :
			;
			break;
		case(3) :
			;
			break;
		case(4) :
			;
			break;
		case(5) :
			;
			break;
		case(6) :
			;
			break;
		case(7) :
			;
			break;
		case(8) :
			;
			break;
		case(9) :
			;
			break;
		case(10) :
			;
			break;
		default :
			break;
	}
}
/* function名表示器 */
void printFunc(unsigned int function) {
	switch(function) {
		case(1) :
			;
			break;
		case(2) :
			;
			break;
		case(3) :
			;
			break;
		case(4) :
			;
			break;
		case(5) :
			;
			break;
		case(6) :
			;
			break;
		case(7) :
			;
			break;
		case(8) :
			;
			break;
		case(9) :
			;
			break;
		case(10) :
			;
			break;
		default :
			break;
	}
}


void printErrorAccessToIncorrectAddr(unsigned int pc, unsigned int instruction, unsigned int *memInit, unsigned int address, unsigned long long breakCount) {
	if (address > MEMORYSIZE) {
		fprintf(stderr, "[ ERROR ]\tMEMORY OVERFLOW (pc:0x%X, instruction:0x%X, address:0x%X, count:%llu)", pc, instruction, address, breakCount);
		exit(1);
	}
	if (memInit[address] == 0 || address > MEMORYSIZE) {
		fprintf(stderr, "[ ERROR ]\tInstruction(0x%X) accessed to uninitialized address: %08X@%llu\n", instruction, address, breakCount);
		exit(1);
	}
}

