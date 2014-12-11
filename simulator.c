#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <sys/time.h>
#include "const.h"
#include "fpu/C/fpu.h"
#include "print.h"
#include "alu.h"
#include "decoder.h"

double getProcTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return( (double) tv.tv_sec + (double) tv.tv_usec * 0.000001 );
}

unsigned int getFileSize(const char fileName[])
{
	unsigned int fsize;

	FILE *fp = fopen(fileName,"rb"); 
 
	/* ファイルサイズを調査 */ 
	fseek(fp,0,SEEK_END); 
	fsize = ftell(fp); 
 
	fclose(fp);
 
	return fsize;
}



/* --hide フラグセット時のfunct関数 */
unsigned int functHide (unsigned int pc, unsigned int instruction, int* flag, unsigned int* reg, unsigned int* opNum, unsigned int* labelRec) {
	unsigned int function, rs, rt, rd, shamt;

	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	rd = (instruction >> 11 ) & 0x1F;
	shamt = (instruction >> 6 ) & 0x1F;
	
	function = instruction & 0x3F;
	switch (function) {
		case (JR) :
			pc = reg[rs];
			labelRec[pc]++;
//			flag[JUMPFLG] = 1;
			opNum[128+JR]++;
			break;
		case (ADDU) :	// rd=rs+rt
			pc = pc + 4;
			reg[rd] = addu(reg[rs], reg[rt]);
			opNum[128+ADDU]++;
			break;
		case (SUBU) :	// rd=rs-rt
			pc = pc + 4;
			reg[rd] = subu(reg[rs], reg[rt]);
			opNum[128+SUBU]++;
			break;
		case (SLT) :
			pc = pc + 4;
			reg[rd] = slt(reg[rs], reg[rt]);
			opNum[128+SLT]++;
			break;
		case (SLL) :
			pc = pc + 4;
/*			if(rd == 0) {
				opNum[128+NOP]++;
				break;
			}
*/			reg[rd] = sll(reg[rt], shamt);
			opNum[128+SLL]++;
			break;
		case (SRL) :
			pc = pc + 4;
/*			if(rd == 0) {
				opNum[128+NOP]++;
				break;
			}
*/			reg[rd] = srl(reg[rt], shamt);
			opNum[128+SRL]++;
			break;
		case (SRA) :
			pc = pc + 4;
			reg[rd] = sra(reg[rt], shamt);
			opNum[128+SRA]++;
			break;
		case (AND) :
			reg[rd] = and(reg[rs], reg[rt]);
			pc = pc + 4;
			opNum[128+AND]++;
			break;
		case (OR) :
			reg[rd] = or(reg[rs], reg[rt]);
			pc = pc + 4;
			opNum[128+OR]++;
			break;
		default :
			fprintf(stderr, "[ ERROR ]\tUnknown switch has selected.(function: 0x%X / pc: 0x%X)\n", function, pc);
			pc = pc + 4;
			flag[UNKNOWNFUNC]++;
	}
	return pc;
}

/* opcodeが0の時の操作を、末尾6ビットによって決める */
unsigned int funct (unsigned int pc, unsigned int instruction, int* flag, unsigned int* reg, unsigned int* opNum, unsigned int* labelRec) {
	unsigned int function = 0;
	unsigned int rs=0;
	unsigned int rs_original;
	unsigned int rt=0;
	unsigned int rd=0;
	unsigned int shamt=0;

	/* [op] [rs] [rt] [rd] [shamt] [funct] */
	/* 先頭&末尾6bitは0であることが保証済み */
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	rd = (instruction >> 11 ) & 0x1F;
	shamt = (instruction >> 6 ) & 0x1F;

	
	function = instruction & 0x3F;
	if(instruction != 0 && flag[HIDEIND] == 0) { 
		printf("\t[function:%2X]\n", function); 
		printf("\t[OPERAND] rs:%08X rt:%08X rd:%08X shamt:%08X \n", rs,rt,rd,shamt);
	}
	switch (function) {
		case (JR) :
			pc = reg[rs];
			labelRec[pc]++;
			printf("\tJR : $ra(%u) = 0x%X / (jump_to) -> 0x%X\n", rs, reg[rs], pc); 
//			flag[JUMPFLG] = 1;
			opNum[128+JR]++;
			break;
		case (ADDU) :	// rd=rs+rt
			rs_original = reg[rs];
			reg[rd] = addu(reg[rs], reg[rt]);
			printf("\tADDU :\t[$%2u: 0x%2X] + [$%2u: 0x%2X] => [$%2u: 0x%2X]\n", rs, rs_original, rt, reg[rt], rd, reg[rd]);
			opNum[128+ADDU]++;
			break;
		case (SUBU) :	// rd=rs-rt
			rs_original = reg[rs];
			reg[rd] = subu(reg[rs], reg[rt]);
			printf("\tSUBU :\t[$%2u 0x%2X], [$%2u 0x%2X] => [rd:%u] 0x%2X\n", rs, rs_original, rt, reg[rt], rd, reg[rd]);
			opNum[128+SUBU]++;
			break;
		case (SLT) :
			printf("\tSLT :\t?([$%2u: 0x%04x] < [$%2u: 0x%04x]) ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				printf("\t=> [$%2u: 0x%4X] <TRUE>\n", rd, reg[rd]);
			} else {
				printf("\t=> [$%2u: 0x%4X] <FALSE>\n", rd, reg[rd]);
			}
			opNum[128+SLT]++;
			break;
		case (SLL) :
			if(rd == 0) {
//				printf("\tNOP\n");
				opNum[128+NOP]++;
			}
			reg[rd] = sll(reg[rt], shamt);
			printf("\tSLL :\t%u << %u -> %u\n", reg[rt], shamt, reg[rd]);
			opNum[128+SLL]++;
			break;
		case (SRL) :
			if(rd == 0) {
				opNum[128+NOP]++;
			}
			reg[rd] = srl(reg[rt], shamt);
			printf("\tSRL :\t%u << %u -> %u\n", reg[rt], shamt, reg[rd]);
			opNum[128+SRL]++;
			break;
		case (SRA) :
			pc = pc + 4;
			reg[rd] = sra(reg[rt], shamt);
			opNum[128+SRA]++;
			break;
		case (AND) :
			reg[rd] = and(reg[rs], reg[rt]);
			printf("\t[$%2u:0x%4X] AND? [$%2u:0x%4X] => [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			opNum[128+AND]++;
			break;
		case (OR) :
			reg[rd] = or(reg[rs], reg[rt]);
			printf("\tOR :[$%2u:0x%4X] OR? [$%2u:0x%4X] => [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			opNum[128+OR]++;
			break;
		default :
			fprintf(stderr, "[ ERROR ]\tUnknown switch has selected.(function: %X/ pc: %X)\n", function, pc);
			flag[UNKNOWNFUNC]++;
	}
	return pc;
}

/* デコーダ */
unsigned int decoderOld (unsigned int pc, unsigned int instruction, unsigned int* memory, unsigned int* memInit, unsigned char* input, unsigned char* srOut, unsigned long long breakCount, int* flag, unsigned int* reg, unsigned int* fpreg, unsigned int* opNum, unsigned int* fpuNum, unsigned int* labelRec, FILE* soFile) {
	unsigned int opcode;
	unsigned int rt;
	unsigned int rs;
	unsigned int rs_original;
	unsigned int im;			// <- 何で符号付きint？
	unsigned int jump=0;
	unsigned int address=0;
	unsigned int alutemp=0;
	static long srInCount = 0;
	static long srOutCount = 0;
	unsigned int writeCond;
	  int temp;
	  int diff = 0; //diff はtempの符号拡張

	opcode = instruction >> 26;	// opcode: 6bitの整数
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	im = instruction & 0xFFFF;

	if(instruction != 0 && flag[HIDEIND] == 0) {
		printf("[ops: %06llu,pc: 0x%x]\n", breakCount, pc);
		printf("[instruction: 0x%2X]\n", instruction);
		printf("\t[opcode:%2X]\n", opcode);
	}
	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) {
		pc = funct(pc, instruction, flag, reg, opNum, labelRec);
	} else if (opcode == FPU) {
		printf("\tFPU \n");
		pc = fpu(pc, instruction, reg, fpreg, flag, fpuNum, labelRec);
	} else if (opcode == ADDIU) {
		im = signExt(im);
		rs_original = reg[rs];
		reg[rt] = (addiu(reg[rs], im) & 0xFFFFFFFF);	// 論理積をとる必要性?
		printf("\tADDIU :\t[$%2u: 0x%4X] + [im: 0x%8X] => [$%2u: 0x%4X]\n", rs, rs_original, im, rt, reg[rt]);
		opNum[ADDIU]++;
	} else if (opcode == JUMP) {
		flag[JUMPFLG] = 1;
		jump = instruction & 0x3FFFFFF;
		pc = jump*4 + PCINIT;	// jumpはpc形式
		labelRec[pc]++;		
		printf("\tJ :\t(jump_to) 0x%04x\n", pc);
		opNum[JUMP]++;
	} else if (opcode == BEQ) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
		if(flag[HIDEIND] == 0) { printf("\tBEQ :\t?([$%2u 0x%2X]=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + diff*4); }
		if(reg[rs] == reg[rt]) {
			flag[JUMPFLG] = 1;
			pc = pc + 4 + diff*4;		// pc形式
			labelRec[pc]++;
//			printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
			if(flag[HIDEIND] == 0) { printf("\t\t<TRUE & JUMP> -> (jump_to) 0x%04x\n", pc); }
		} else {
			if(flag[HIDEIND] == 0) { printf("\t<FALSE & NOP>\n"); }
		}
		opNum[BEQ]++;
	} else if (opcode == BNE) { // bne I-Type: 000101 rs rt BranchAddr 	等しくないなら分岐 
		temp = instruction & 0xFFFF;
		diff = (temp >= (1<<15)) ? temp-(1<<16) : temp; //diff はtempの符号拡張
		if(flag[HIDEIND] == 0) { printf("\tBNE :\t?([$%2u 0x%2X]!=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + diff*4); }
		if(reg[rs] != reg[rt]) {
		  flag[JUMPFLG] = 1;
		  pc = pc + 4 + diff*4;		// pc形式
		  labelRec[pc]++;
//		  printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
		  if(flag[HIDEIND] == 0) {
			printf("\t\t<TRUE & JUMP> -> (jump_to) 0x%04x\n", pc);
		  }
		} else {
		  if(flag[HIDEIND] == 0) {
			printf("\t<FALSE & NOP>\n");
		  }
		}
		opNum[BNE]++;
	} else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
		opNum[LW]++;
		im = signExt(im);
		address = (reg[rs]+im);

		/* Memory mapped I/O対応 */
		if( (address & MMIO) == MMIO) {
			if(address == MMIOREADRDY) {
				if (flag[INPUTSIZE] > srInCount) {
					reg[rt] = 1;
				} else {
					pc = flag[MAXPC];
					flag[JUMPFLG] = 1;
				}
				printf("\tMMIOREAD_Ready :\t [$%2u] <- %u\n", rt, reg[rt]);
			} else if (address == MMIOREAD) {
				reg[rt] = input[srInCount];
				srInCount++;
				printf("\tMMIOREAD(%2ld) :\t [$%2u] <- 0x%X\n", srInCount, rt, reg[rt]);
			} else if (address == 0xFFFF0008) {
				reg[rt] = 1;
				printf("\tMMIOWRITE_Ready :\t [$%2u] <- %u\n", rt, reg[rt]);
			} else {
				fprintf(stderr, "[ ERROR ]\tirregular code(0x%X).\n", address);
				exit(1);
			}
		} else {
			if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
				fprintf(stderr, "[ ERROR ] Memory overflow\n");
				exit(1);
			}
			if (memInit[address] == 0) {
				fprintf(stderr, "[ ERROR ]\tAccessed to uninitialized address: %08X\n", address);
				exit(1);
			}

			reg[rt] = lw(address, memory);
			printf("\tLW :\tmemory[address:0x%04X]=0x%4X \n", address, memory[address]);
			printf("\t\treg[rt(%u)] = 0x%X\n", rt, reg[rt]);
		}
	} else if (opcode == SW) {
		// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm番地に書き込む
		opNum[SW]++;
		im = signExt(im);
		address = (reg[rs]+im);

		/* Memory mapped I/O対応 */
		if( (address & MMIO) == MMIO) {
			if(flag[HIDEIND] == 0) { printf("\t(im:0x%X)/(reg[%2u]:0x%X)/(reg[%2u]:0x%X)\n", im, rs, reg[rs], rt, reg[rt]); }
			writeCond = fwrite(&reg[rt], sizeof(unsigned char), 1, soFile);
			if(srOutCount < FILESIZE) {
				srOut[srOutCount] = reg[rt] & 0xFF;
				flag[OUTPUTSIZE]++;
				srOutCount++;
			} else {
				fprintf(stderr, "[ ERROR ]\tserial port size is over %d", FILESIZE);
			}
			if(writeCond == 0) { fprintf(stderr, "failed to fwrite at \n"); }
			if(flag[HIDEIND] == 0) {
				printf("\tMMIOWRITE:\twrite to serialport (%u).\n", reg[rt]);
			}
		} else {
			if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
				fprintf(stderr, "[ ERROR ] Memory overflow\n");
				exit(1);
			}
			if(flag[HIDEIND] == 0) { printf("\tSW :\t[address: 0x%04X-0x%04X] <- [$%2u(rt): 0x%2X]\n", address, address+3, rt, reg[rt]); }
			if(address > MEMORYSIZE) {
				fprintf(stderr, "[ ERROR ]\tMemory overflow\n");
				exit(1);
			}
			sw(reg[rt], address, memory);
			if(flag[HIDEIND] == 0) { printf("\tmemory[address: 0x%04X-0x%04X] : %02X %02X %02X %02X\n", address, address+3, memory[address+3], memory[address+2], memory[address+1], memory[address]); }
		}
	} else if (opcode == LUI) {
		/* 
			Description: The immediate value is shifted left 16 bits and stored in the register. The lower 16 bits are zeroes.
			Operation:	$t = (imm << 16); advance_pc (4);
			Syntax:		lui $t, imm 
		*/
		reg[rt] = im << 16;
		if(flag[HIDEIND] == 0) { printf("\tLUI :\tReg[$%2u] <- [0x%4X (imm)]\n", rt, im); }
		opNum[LUI]++;
	} else if (opcode == JAL) {
		jump = instruction & 0x3FFFFFF;
		flag[JUMPFLG] = 1;
		if(flag[HIDEIND] == 0) { printf("\tJAL: (jump_to) 0x%04x ($ra <- pc[%X])\n", jump*4, pc+4); }
		reg[31] = pc+4;
		pc = jump*4 + PCINIT;
		opNum[JAL]++;
		labelRec[pc]++;
//		printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
	} else if (opcode == ORI) {
		opNum[ORI]++;
		alutemp = reg[rs];
		reg[rt] = ori(reg[rs], im);
		if(flag[HIDEIND] == 0) { printf("\tOR: [reg($%02u):0x%4X] OR? [imm:0x%4X] => [reg($%02u)] 0x%2X\n", rs, alutemp, im, rt, reg[rt]); }
	} else if (opcode == ANDI) {
		/* R[rt] = R[rs] & ZeroExtImm */
		opNum[ANDI]++;
		reg[rt] = reg[rs] & signExt(im);
		if(flag[HIDEIND] == 0) { printf("\tANDI: [$%2u:0x%4X] ANDi? [im:0x%4X] => [$%2u] 0x%4X\n", rs, reg[rs], im, rt, reg[rt]); }
	} else {
		printf("\t[Unknown OPCODE] 0x%X\n", opcode);
		flag[UNKNOWNOP]++;
	}

	if(flag[JUMPFLG] == 0) {
		pc = pc + 4;
		flag[JUMPFLG] = 0;
	} else {
		flag[JUMPFLG] = 0;
	}
	return pc;
}

int main (int argc, char* argv[]) {
	int flag[REGSIZE] = { 0 };
	unsigned int* opBuff;	// ファイルから読み込む命令列
	int fd1 = 0;
	unsigned int maxpc;
	unsigned int pc = 0;
	unsigned int operation;	// 実行中命令
	unsigned int opcode;
	unsigned long long breakCount = 0;
	unsigned long long breakpoint = 0xFFFFFFFFFFFFFF;
	unsigned int breakShift = 40;	//  
	unsigned char *input;
	unsigned char *srOut;
	unsigned int *memory;
	unsigned int *memInit;

	unsigned int reg[REGSIZE], fpreg[REGSIZE];	// 32 register
	int i = 0;
	unsigned int count = 0;
	unsigned int snum = 0;	// serial portに書き出したbyte数
	unsigned int srCount=0;
	unsigned int mstart = 0, mfinish = MEMORYSIZE;
	unsigned int fpuNum[OPNUM] = { 0 };	//各浮動小数点命令の実行回数 fpuNum[OPCODE]++ の形で使用
	unsigned int opNum[OPNUM] = { 0 };	//各命令の実行回数 opNum[OPCODE] の形で使用/function系はopNum[FUNCCODE+128]
	unsigned int labelRec[BUFF] = { 0 };
//	unsigned long long labelCount = 1;

	char sequentialBuff[BUFF];
	FILE* soFile;
	unsigned int fSize;
	int comp;
	double timebuff[BUFF];
	int i_time = 0;
	

	timebuff[i_time] = getProcTime();
//	fprintf(stderr, "time[%2d] = %f\n", i_time, timebuff[i_time]);
/*
	for(count=0;count<BUFF;count++) {
		labelRec[count]=0;
	}
*/


	count=0;
	opBuff = (unsigned int *) calloc( FILESIZE, sizeof(unsigned int) );
	if(opBuff == NULL) {
		perror("memory allocation error (opBuff)\n");
		return -1;
	}
	input = (unsigned char *) calloc( FILESIZE, sizeof(unsigned char) );
	if(input == NULL) {
		perror("memory allocation error (input)\n");
		return -1;
	}
	memory = (unsigned int *) calloc( MEMORYSIZE, sizeof(unsigned int) );
	if(memory == NULL) {
		perror("memory allocation error (memory)\n");
		return -1;
	}
	memInit = (unsigned int *) calloc( MEMORYSIZE, sizeof(unsigned int) );
	if(memInit == NULL) {
		perror("memory allocation error (memInit)\n");
		return -1;
	}
	srOut = (unsigned char *) calloc( MEMORYSIZE, sizeof(unsigned char) );
	if(srOut == NULL) {
		perror("memory allocation error (srOut)\n");
		return -1;
	}

	/* 引数として<ファイル:プログラム>をとる。なければ強制終了 */
	if (argc < 2) {
		printhelp();
		return -1;
		printf("please input file.\n");
	}
	/* ファイルから実行命令列を読み込む */
	printf("%s\n", argv[1]);
	fd1 = open(argv[1], O_RDONLY);
	maxpc = read(fd1, opBuff, BUFF);

	fSize = getFileSize(argv[1]);
	fprintf(stderr, "fileSize = %u (Bytes) / maxpc : %u\n", fSize, maxpc);
	if(maxpc==0) {
		perror("[ ERROR ]\tfailed to read input file.\n");
		exit(1);
	}
	soFile = fopen("serial.out", "wb");
	if(soFile == NULL) {
		fprintf(stderr, "[ ERROR ]\tCannot make \"serial.out\".\n");
		exit(1);
	}

	printf("Commandline Option: ");
	/* 引数の処理 */
	/*	色々終わったら手をつける
		procArg(argc, argv, flag);
	*/
	i = 2;
//	fprintf(stderr, "%d\n", argc);
	while(i < argc) {
		/* help */
		comp = strcmp(argv[i], HELP);
		if(comp == 0) { printhelp(); exit(1); }

		/* hide */
		comp = strcmp(argv[i], HIDE);
		if(comp == 0) { 
			flag[HIDEIND] = 1;
			printf("HIDE, ");
		}

		/* printreg */
		comp = strcmp(argv[i], PRINTREG);
		if(comp == 0) { 
			flag[PRINTREGIND] = 1;
			printf("PRINTREG, ");
		}
		/* breakpoint */
		comp = strcmp(argv[i], BREAKPOINT);
		if(comp == 0 && argc >= i+1) {
			breakpoint = (unsigned long long) atoi(argv[i+1]);
			printf("BREAKPOINT = %llu, ", breakpoint);
		}
		/* sequential */
		comp = strcmp(argv[i],SEQUENTIAL);
		if(comp == 0) {
			flag[5] = 1;
			printf("SEQUENTIAL, ");
		}
		/* print_mem */
		comp = strcmp(argv[i],PRINTMEM);
		if((comp == 0) && (argc > (i+2))) { 
			flag[6] = 1;
			mstart = (unsigned int) atoi(argv[i+1]);
			mfinish = (unsigned int) atoi(argv[i+2]);
			printf("PRINTMEM, ");
			i = i + 2;
		}
		/* native */
		comp = strcmp(argv[i],FPUNATIVE);
		if((comp == 0)) {
			flag[7] = 1;
			printf("FPUNATIVE, ");
		}
		/* serialout */
		comp = strcmp(argv[i],SERIALOUT);
		if((comp == 0) && (argc > (i+1)) && flag[8] != 1) {
			flag[8] = 1;
			if(remove("serial.out") == 0) { ; }
			fclose(soFile);
			printf("SERIALOUT(\"%s\"), ", argv[i+1]);
			soFile = fopen(argv[i+1], "wb");
			if(soFile == NULL) {
				fprintf(stderr, "[ ERROR ]\tCannot make \"%s\".\n", argv[i+1]);
				exit(1);
			}
		}
		i++;
	}
	i_time++;
	timebuff[i_time] = getProcTime();
	fprintf(stderr, "time[%2d] = %.12f\n", i_time, (timebuff[i_time] - timebuff[0]) );


	i=0;
	count=0;
	flag[SDATA] = 0;
	while(count*4 < maxpc) {
		if (opBuff[count] == 0xFFFFFFFF) {
			break;
		}
		if ( (opBuff[count] != 0) && (flag[SDATA] == 0) ) {
			memory[4*count+3+PCINIT] = opBuff[count] & 0xFF;
			memory[4*count+2+PCINIT] = (opBuff[count] >> 8) & 0xFF;
			memory[4*count+1+PCINIT] = (opBuff[count] >> 16) & 0xFF;
			memory[4*count+PCINIT]   = opBuff[count] >> 24;
		}
		memInit[4*count+3+PCINIT] = 1;
		memInit[4*count+2+PCINIT] = 1;
		memInit[4*count+1+PCINIT] = 1;
		memInit[4*count+PCINIT]   = 1;
		if(count*4 > MEMORYSIZE) { fprintf(stderr, "[ ERROR ]\tMemory overflow.\n"); exit(1); }
		if(count*4 > BLOCKRAM) { fprintf(stderr, "[ INFO ]\tProgram has reached at BLOCKRAM.\n"); count++; break; }
		count++;
	}
	flag[SDATA] = 1;
	count++;
	maxpc=count*4;
	fSize = fSize - count*4;
	fprintf(stderr, "\nmaxpc:0x%X(%uline)\n", maxpc+PCINIT, (maxpc)/4);

	/* NOPを32個挿入 */
	for(i=0; i<32*4; i++) {
		memory[4*count+i+PCINIT]  = 0;
		memInit[4*count+i+PCINIT] = 1;
	}

	/* シリアルポートからのデータ列をinput[]に格納 */
	srCount=0;
	while(count*4 < FILESIZE && count < BUFF) {
		input[4*srCount] = opBuff[count] & 0xFF;
		input[4*srCount+1] = (opBuff[count] >> 8) & 0xFF;
		input[4*srCount+2] = (opBuff[count] >> 16) & 0xFF;
		input[4*srCount+3] = opBuff[count] >> 24;
		if(opBuff[count] != 0) {
			if (input[4*srCount+3] != 0) {
				flag[INPUTSIZE] = 4*srCount+3;
			} else if (input[4*srCount+2] != 0) {
				flag[INPUTSIZE] = 4*srCount+2;
			} else if (input[4*srCount+1] != 0) {
				flag[INPUTSIZE] = 4*srCount+1;
			} else if (input[4*srCount] != 0) {
				flag[INPUTSIZE] = 4*srCount;
			}
		}
		srCount++;
		count++;
	}
	if(flag[INPUTSIZE] != 1) { flag[INPUTSIZE]++; }
	fprintf(stderr, "INPUTSIZE: %u / fSize: %u\n", flag[INPUTSIZE], fSize);
	/* 初期化ここまで */
	i=0;
	count=0;

	/* initialize */
	/* register init */
	for(i=0; i<REGSIZE; i++) {
		reg[i] = 0;
		fpreg[i] = 0;
	}
	for(i=0;i<OPNUM;i++) {
		opNum[i]=0;
//		opNum[128+i]=0;
	}
	reg[REGSP] = 0x1FFF4;

	/* Program Counter init */
	pc = PCINIT;
	flag[MAXPC] = maxpc + 4;
	fprintf(stderr, "flag[HIDEIND] = %u\n", flag[HIDEIND]);

	i_time++;
	timebuff[i_time] = getProcTime();
	fprintf(stderr, "time[%2d] = %.12f (<-<-<- Initialize <-<-<-)\n", i_time, (timebuff[i_time] - timebuff[0]) );
	if(flag[HIDEIND] == 0) { printf("\n== next: %u ==\n", ((pc-PCINIT)/4)); }

	/* シミュレータ本体 */
	while(pc < maxpc+PCINIT+1 && breakCount < breakpoint+1) {	// unsigned int
		reg[0] = 0;
		operation = memory[pc] | memory[pc+1] << 8 | memory[pc+2] << 16 | memory[pc+3] << 24;
		opcode = operation >> 26;
		switch (opcode) {
			case(FPU) :
				pc = fpuHide(pc, operation, reg, fpreg, fpuNum, labelRec);
				break;
			case(0) :
				pc = functHide(pc, operation, flag, reg, opNum, labelRec);
				break;
			default :
				pc = decoder(pc, operation, memory, memInit, input, srOut, breakCount, flag, reg, opNum, labelRec, soFile, opcode);
				break;
		}
		breakCount++;
		if( (breakCount << breakShift) == 0) { 
			fprintf(stderr, "[ INFO ]\tprocessing instructions... @ %10llX\n", breakCount);
			i_time++;
			timebuff[i_time] = getProcTime();
			fprintf(stderr, "\t\ttotal time[%2d]: %.6f\n", i_time, (timebuff[i_time] - timebuff[0]) );
			if(breakShift > 32) {
				breakShift--;
			}
		}
		if (flag[HIDEIND] == 1) {
			;
		} else if ((flag[HIDEIND] == 0) && flag[PRINTREGIND] == 1 && operation != 0) {
			printRegister(reg);	// 命令実行後のレジスタを表示する
			if(flag[HIDEIND] == 0) { printf("\n== next: %u ==\n", ((pc-PCINIT)/4)); }
		} else if(flag[5] == 1) { 
			if (fgets(sequentialBuff, sizeof(sequentialBuff) - 1, stdin) == NULL) { ; }
		} else if(flag[HIDEIND] == 0) {
			 printf("\n== next: %u ==\n", ((pc-PCINIT)/4));
		}
	}

	fprintf(stderr, "[ INFO ]\ttotal processed instructions... @ %10llX\n", breakCount);
	i_time++;
	timebuff[i_time] = getProcTime();
	fprintf(stderr, "\t\ttime[%2d] = %.8f\n", i_time, (timebuff[i_time] - timebuff[0]) );

	snum = mstart;
	if ((mfinish - mstart) > 0) {
		printf("\nメモリダンプ:\n");
		while(snum%4 != 0) {
			snum++;
		}
	}
	while((snum < MEMORYSIZE-3) && (snum < mfinish)) {
		if(snum < maxpc+PCINIT) {
			snum = snum + 4;
			continue;
		}
		if ( (memory[snum] != 0 || memory[snum+1] != 0 || memory[snum+2] != 0 || memory[snum+3] != 0) ) {
			printf("memory[0x%06X] = %02X %02X %02X %02X\n", snum, memory[snum+3], memory[snum+2], memory[snum+1], memory[snum]);
		}
		snum += 4;
	}

	printf("\nレジスタ吐き出し:\n");
	printRegister(reg);

	printf("\n\nTotal instructions: \n\t%llu\n", breakCount);
	printf("Total instructions (except NOP): \n\t%llu\n", breakCount - opNum[128+NOP]);
	printf("\n(OP)\t: \t(Num), \t(Ratio)\n");
	if(opNum[128+NOP] != 0) { printf("NOP 	: %6u, %05.2f (%%)\n", opNum[128+NOP], (double) 100*opNum[128+NOP]/breakCount); }
	printOpsCount(opNum, fpuNum, breakCount);
	printf("\nラベル呼び出し先:\n\t");
	snum = 0;
	count = 0;
	while(snum < maxpc+PCINIT) {
		if (labelRec[snum] != 0) {
			printf("Line: %6u -> %12u (回)\n\t",(snum-PCINIT)/4,labelRec[snum]);
			count++;
		}
		snum++;
	}
	printf("\n");


	if(flag[UNKNOWNOP] != 0) {
		fprintf(stderr, "[ ERROR ]\tUnknown opcode existed! (%u)\n", flag[UNKNOWNOP]);
		printf("[ ERROR ]\tUnknown opcode existed!\n");
		exit(1);
	}
	if(flag[UNKNOWNFUNC] != 0) {
		fprintf(stderr, "[ ERROR ]\tUnknown function code existed! (%u)\n", flag[UNKNOWNFUNC]);
		printf("[ ERROR ]\tUnknown function code existed!\n");
		exit(1);
	}
	i=0;
	count = 0;
	/* 終了処理 */
	free(memory);
	memory = NULL;
	free(srOut);
	srOut = NULL;
	free(opBuff);
	opBuff = NULL;
	free(input);
	input = NULL;
	free(memInit);
	memInit = NULL;
	i_time++;
	timebuff[i_time] = getProcTime();
	fprintf(stderr, "total time = %.3f\n", (timebuff[i_time] - timebuff[0]));

	if(soFile != NULL) {
		fclose(soFile);
	}
	close(fd1);
	return 0;
}

