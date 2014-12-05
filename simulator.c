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


/* store word */
// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm(符号拡張)番地に書き込む ☆sw()呼び出し時点で拡張済み
/* リトルエンディアン */
unsigned int sw(unsigned int rt, unsigned int address, unsigned int* memory) {
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int addr3;


	addr3 = (rt & 0xFF000000) >> 24;
	addr2 = (rt & 0x00FF0000) >> 16;
	addr1 = (rt & 0x0000FF00) >>  8;
	addr0 = rt & 0x000000FF;

/*	if (stackPointer == 31) {
		memory[address+3] = addr3;
		memory[address+2] = addr2;
		memory[address+1] = addr1;
		memory[address] = addr0;
	} else {
		memory[address+3] = addr3;
		memory[address+2] = addr2;
		memory[address+1] = addr1;
		memory[address] = addr0;
	}
*/
		memory[address+3] = addr3;
		memory[address+2] = addr2;
		memory[address+1] = addr1;
		memory[address] = addr0;

	if(rt != ( memory[address] | memory[address+1] << 8 | memory[address+2] << 16 | memory[address+3] << 24 )) {
		fprintf(stderr, "\n[ ERROR ]\tFailed to SW\n");
		exit(1);
	}
	return 0;
}
/* load word */
// lw rs,rt,Imm => R(rt) <- M[ R(rs)+Imm ]	メモリのR(rs)+Imm番地の内容をrtに書き込む
unsigned int lw(unsigned int address, unsigned int* memory) {
	unsigned rt = 0;
	rt = memory[address] | memory[address+1] << 8 | memory[address+2] << 16 | memory[address+3] << 24;
	return rt;
}


/* --hide フラグセット時のfunct関数 */
unsigned int functHide (unsigned int pc, unsigned int instruction, int* flag, unsigned int* reg, unsigned int* opNum, unsigned int* labelRec) {
	unsigned int function = 0;
	unsigned int rs=0;
	unsigned int rt=0;
	unsigned int rd=0;
	unsigned int shamt=0;
	unsigned int im=0;
	if (im == 0) { ; }

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
			if(rd == 0) {
				opNum[128+NOP]++;
				break;
			}
			reg[rd] = sll(reg[rt], shamt);
			opNum[128+SLL]++;
			break;
		case (SRL) :
			pc = pc + 4;
			if(rd == 0) {
				opNum[128+NOP]++;
				break;
			}
			reg[rd] = srl(reg[rt], shamt);
			opNum[128+SRL]++;
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
			fprintf(stderr, "Unknown switch has selected.\n");
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
	unsigned int im=0;
	if (im == 0) { ; }

	/* [op] [rs] [rt] [rd] [shamt] [funct] */
	/* 先頭&末尾6bitは0であることが保証済み */
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	rd = (instruction >> 11 ) & 0x1F;
	shamt = (instruction >> 6 ) & 0x1F;

	if(instruction != 0 && flag[HIDEIND] != 1) {
		printf("\t[OPERAND] rs:%08X rt:%08X rd:%08X shamt:%08X \n", rs,rt,rd,shamt);
	}
	
	function = instruction & 0x3F;
	if(instruction != 0 && flag[HIDEIND] != 1) { printf("\t[function:%2X]\n", function); }
	switch (function) {
		case (JR) :
			pc = reg[rs];
			labelRec[pc]++;
			if(flag[HIDEIND] != 1) { 
//				printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
				printf("\tJR : $ra(%u) = 0x%X / (jump_to) -> 0x%X\n", rs, reg[rs], pc); 
			}
			flag[JUMPFLG] = 1;
			opNum[128+JR]++;
			break;
		case (ADDU) :	// rd=rs+rt
			rs_original = reg[rs];
			reg[rd] = addu(reg[rs], reg[rt]);
			if(flag[HIDEIND] != 1) { printf("\tADDU :\t[$%2u: 0x%2X] + [$%2u: 0x%2X] => [$%2u: 0x%2X]\n", rs, rs_original, rt, reg[rt], rd, reg[rd]); }
			opNum[128+ADDU]++;
			break;
		case (SUBU) :	// rd=rs-rt
			rs_original = reg[rs];
			reg[rd] = subu(reg[rs], reg[rt]);
			if(flag[HIDEIND] != 1) { printf("\tSUBU :\t[$%2u 0x%2X], [$%2u 0x%2X] => [rd:%u] 0x%2X\n", rs, rs_original, rt, reg[rt], rd, reg[rd]); }
			opNum[128+SUBU]++;
			break;
		case (SLT) :
			if(flag[HIDEIND] != 1) { printf("\tSLT :\t?([$%2u: 0x%04x] < [$%2u: 0x%04x]) ", rs, reg[rs], rt, reg[rt]); }
			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				if(flag[HIDEIND] != 1) { printf("\t=> [$%2u: 0x%4X] <TRUE>\n", rd, reg[rd]); }
			} else {
				if(flag[HIDEIND] != 1) { printf("\t=> [$%2u: 0x%4X] <FALSE>\n", rd, reg[rd]); }
			}
			opNum[128+SLT]++;
			break;
		case (SLL) :
			if(rd == 0) {
				if(flag[HIDEIND] != 1) { printf("\tNOP\n"); }
				opNum[128+NOP]++;
				break;
			}
			reg[rd] = sll(reg[rt], shamt);
			if(flag[HIDEIND] != 1) { printf("\tSLL :\t%u << %u -> %u\n", reg[rt], shamt, reg[rd]); }
			opNum[128+SLL]++;
			break;
		case (SRL) :
			if(rd == 0) {
				if(flag[HIDEIND] != 1) { printf("\tNOP\n"); }
				opNum[128+NOP]++;
				break;
			}
			reg[rd] = srl(reg[rt], shamt);
			if(flag[HIDEIND] != 1) { printf("\tSRL :\t%u << %u -> %u\n", reg[rt], shamt, reg[rd]); }
			opNum[128+SRL]++;
			break;
		case (AND) :
			reg[rd] = and(reg[rs], reg[rt]);
			if(flag[HIDEIND] != 1) { printf("\t[$%2u:0x%4X] AND? [$%2u:0x%4X] => [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]); }
			opNum[128+AND]++;
			break;
		case (OR) :
			reg[rd] = or(reg[rs], reg[rt]);
			if(flag[HIDEIND] != 1) { printf("\t[$%2u:0x%4X] OR? [$%2u:0x%4X] => [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]); }
			opNum[128+OR]++;
			break;
		default :
			printf("Unknown switch has selected.\n");
			flag[UNKNOWNFUNC]++;
	}
	return pc;
}

/* --hide フラグセット時のデコーダ */
unsigned int decoderHide (unsigned int pc, unsigned int instruction, unsigned int* memory,unsigned char* input, unsigned char* srOut, int* flag, unsigned int* reg, unsigned int* opNum, unsigned int* labelRec, FILE* soFile) {
	unsigned int opcode, rt, rs, address;
	int im;			// <- 何で符号付きint？
	unsigned int jump;
	static long srInCount = 0;
	static long srOutCount = 0;
	int temp, diff;

	opcode = instruction >> 26;	// opcode: 6bitの整数
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	im = instruction & 0xFFFF;

	temp = instruction & 0xFFFF;
	diff = (temp >= (1<<15)) ? temp-(1<<16) : temp; //diff はtempの符号拡張
	pc = pc + 4;

	switch (opcode) {
		case(ADDIU) :
			if( im >= 0x8000 ) {
				im = im | 0xFFFF0000;
			}
			reg[rt] = (addiu(reg[rs], im, rt) & 0xFFFFFFFF);	// 処理部
			opNum[ADDIU]++;
			break;
		case(JUMP) :
			flag[JUMPFLG] = 1;
			jump = instruction & 0x3FFFFFF;
			pc = jump*4 + PCINIT;	// jumpはpc形式
			labelRec[pc]++;		
			opNum[JUMP]++;
			break;
		case(BEQ) : 
			if(reg[rs] == reg[rt]) {
//				flag[JUMPFLG] = 1;
				// 0x48 = 0x20 + 4 + diff*4 +0x0	<-> diff*4 = 0x48-0x24 <-> diff = 9
				temp = instruction & 0xFFFF;
				diff = (temp >= (1<<15)) ? temp-(1<<16) : temp; //diff はtempの符号拡張
				pc = pc + diff*4;		// pc形式
				labelRec[pc]++;
			}
			opNum[BEQ]++;
			break;
		case(BNE) :		// bne I-Type: 000101 rs rt BranchAddr 	等しくないなら分岐 
			if(reg[rs] != reg[rt]) {
//				flag[JUMPFLG] = 1;
				temp = instruction & 0xFFFF;
				diff = (temp >= (1<<15)) ? temp-(1<<16) : temp; //diff はtempの符号拡張
				pc = pc + diff*4;		// pc形式
				labelRec[pc]++;
			} 
			opNum[BNE]++;
			break;
		case(LW) :	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
			opNum[LW]++;

			if( im >= 0x8000 ) {	//imは16bit
				im = im | 0xFFFF0000;
			}
			address = (reg[rs] + im);
	
			// Memory mapped I/O対応
			if( (address & MMIO) == MMIO) {
				if(address == MMIOREADRDY) {
					if (flag[INPUTSIZE] > srInCount) {
						reg[rt] = 1;
					} else {
						pc = flag[MAXPC];
//						flag[JUMPFLG] = 1;
					}
				} else if (address == MMIOREAD) {
					reg[rt] = input[srInCount];
					srInCount++;
				} else if (address == 0xFFFF0008) {
					reg[rt] = 1;
				} else {
					fprintf(stderr, "[ ERROR ]\tirregular code(0x%X).\n", address);
					exit(1);
				}
			} else {
				if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
					fprintf(stderr, "[ ERROR ] Memory overflow\n");
					exit(1);
				}
				reg[rt] = lw(address, memory);
			}
			break;
		case(SW) :
			opNum[SW]++;

			if( im >= 0x8000 ) {	//imは16bit
				im = im | 0xFFFF0000;
			}
			address = (reg[rs] + im);

			if( (address & MMIO) == MMIO) {
				unsigned int writeCond;
				writeCond = fwrite(&reg[rt], sizeof(unsigned char), 1, soFile);
				if(srOutCount < FILESIZE) {
					srOut[srOutCount] = reg[rt] & 0xFF;
					flag[OUTPUTSIZE]++;
					srOutCount++;
				}
				if(writeCond == 0) { fprintf(stderr, "failed to fwrite at \n"); }
			} else {
				if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
					fprintf(stderr, "[ ERROR ]\tMemory overflow\n");
					exit(1);
				}
	//			address = address % MEMORYSIZE;
/*				if(address > MEMORYSIZE) {
					fprintf(stderr, "[ ERROR ]\tMemory overflow\n");
					exit(1);
				}
*/				sw(reg[rt], address, memory);
			}
			break;
		case(LUI) :
			reg[rt] = im << 16;
			opNum[LUI]++;
			break;
		case(JAL) :
			jump = instruction & 0x3FFFFFF;
			flag[JUMPFLG] = 1;
			reg[31] = pc;
			pc = jump*4 + PCINIT;
			opNum[JAL]++;
			labelRec[pc]++;
			break;
		case(ORI) :
			opNum[OR]++;
			reg[rt] = ori(reg[rs], im);
			break;
		case(ANDI) :
			opNum[ANDI]++;
			reg[rt] = reg[rs] & im;
			break;
		default :
			fprintf(stderr, "\t[Unknown OPCODE]\n");
			flag[UNKNOWNOP]++;
	}
	return pc;
}

/* デコーダ */
unsigned int decoder (unsigned int pc, unsigned int instruction, unsigned int* memory,unsigned char* input, unsigned char* srOut, unsigned long long breakCount, int* flag, unsigned int* reg, unsigned int* fpreg, unsigned int* opNum, unsigned int* fpuNum, unsigned int* labelRec, FILE* soFile) {
	unsigned int opcode=0;
	unsigned int rt=0;
	unsigned int rs=0;
	unsigned int rs_original;
	int im=0;			// <- 何で符号付きint？
	unsigned int jump=0;
	unsigned int address=0;
	unsigned int alutemp=0;
	static long srInCount = 0;
	static long srOutCount = 0;
	unsigned int writeCond;

	opcode = instruction >> 26;	// opcode: 6bitの整数
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	im = instruction & 0xFFFF;

	if(instruction != 0 && flag[HIDEIND] != 1) {
		printf("[ops: %06llu,pc: 0x%x]\n", breakCount, pc);
		printf("[instruction: 0x%2X]\n", instruction);
		printf("\t[opcode:%2X]\n", opcode);
	}
	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) {
		if(flag[HIDEIND] == 1) {
			pc = functHide(pc, instruction, flag, reg, opNum, labelRec);
		} else {
			pc = funct(pc, instruction, flag, reg, opNum, labelRec);
		}

	} else if (opcode == FPU) {
		printf("\tFPU \n");
		pc = fpu(pc, instruction, reg, fpreg, flag, fpuNum, labelRec);
	} /* else if (opcode == SRCV) {
		reg[rt] = rrb(srIn);
		if(flag[HIDEIND] != 1) { printf("\tSRCV(rrb):\t reg[%u]:%02X  \n",rt,reg[rt]); }
		opNum[SRCV]++;
	} else if (opcode == SSND) {
		if(flag[HIDEIND] != 1) { printf("\tSSND(rsb):\t \n"); }
		rsb(reg[rt], srOut);
		opNum[SSND]++;
	} */else if (opcode == ADDIU) {
		if( im >= 0x8000 ) {
			im = im | 0xFFFF0000;
//			printf("(%X)",Imm);
		}
		rs_original = reg[rs];
		reg[rt] = (addiu(reg[rs], im, rt) & 0xFFFFFFFF);	// 処理部
		if(flag[HIDEIND] != 1) { printf("\tADDIU :\t[$%2u: 0x%4X] + [im: 0x%8X] => [$%2u: 0x%4X]\n", rs, rs_original, im, rt, reg[rt]); }
		opNum[ADDIU]++;
	} else if (opcode == JUMP) {
		flag[JUMPFLG] = 1;
		jump = instruction & 0x3FFFFFF;
		pc = jump*4 + PCINIT;	// jumpはpc形式
		labelRec[pc]++;		
		if(flag[HIDEIND] != 1) { 
//			printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
			printf("\tJ :\t(jump_to) 0x%04x\n", pc);
		}
		opNum[JUMP]++;
	} else if (opcode == BEQ) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
	  int temp = instruction & 0xFFFF;
	  int diff = (temp >= (1<<15)) ? temp-(1<<16) : temp; //diff はtempの符号拡張
		if(flag[HIDEIND] != 1) { printf("\tBEQ :\t?([$%2u 0x%2X]=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + diff*4); }
		if(reg[rs] == reg[rt]) {
			flag[JUMPFLG] = 1;
			// 0x48 = 0x20 + 4 + diff*4 +0x0	<-> diff*4 = 0x48-0x24 <-> diff = 9
			pc = pc + 4 + diff*4;		// pc形式
			labelRec[pc]++;
//			printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
			if(flag[HIDEIND] != 1) { printf("\t\t<TRUE & JUMP> -> (jump_to) 0x%04x\n", pc); }
		} else {
			if(flag[HIDEIND] != 1) { printf("\t<FALSE & NOP>\n"); }
		}
		opNum[BEQ]++;
	} else if (opcode == BNE) { // bne I-Type: 000101 rs rt BranchAddr 	等しくないなら分岐 
	  int temp = instruction & 0xFFFF;
	  int diff = (temp >= (1<<15)) ? temp-(1<<16) : temp; //diff はtempの符号拡張
	  if(flag[HIDEIND] != 1) { printf("\tBNE :\t?([$%2u 0x%2X]!=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + diff*4); }
		if(reg[rs] != reg[rt]) {
		  flag[JUMPFLG] = 1;
		  pc = pc + 4 + diff*4;		// pc形式
		  labelRec[pc]++;
//		  printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
		  if(flag[HIDEIND] != 1) {
			printf("\t\t<TRUE & JUMP> -> (jump_to) 0x%04x\n", pc);
		  }
		} else {
		  if(flag[HIDEIND] != 1) {
			printf("\t<FALSE & NOP>\n");
		  }
		}
		opNum[BNE]++;
	} else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード
		opNum[LW]++;

		if( im >= 0x8000 ) {	//imは16bit
			im = im | 0xFFFF0000;
		}
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
				if(flag[HIDEIND] != 1) { printf("\tMMIOREAD_Ready :\t [$%2u] <- %u\n", rt, reg[rt]); }
			} else if (address == MMIOREAD) {
				reg[rt] = input[srInCount];
				srInCount++;
				if(flag[HIDEIND] != 1) { printf("\tMMIOREAD(%2ld) :\t [$%2u] <- 0x%X\n", srInCount, rt, reg[rt]); }
			} else if (address == 0xFFFF0008) {
				reg[rt] = 1;
				if(flag[HIDEIND] != 1) { printf("\tMMIOWRITE_Ready :\t [$%2u] <- %u\n", rt, reg[rt]); }
			} else {
				fprintf(stderr, "[ ERROR ]\tirregular code(0x%X).\n", address);
				exit(1);
			}
		} else {
			if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
				fprintf(stderr, "[ ERROR ] Memory overflow\n");
				exit(1);
			}
			if(flag[HIDEIND] != 1) { printf("\tLW :\tmemory[address:0x%04X]=0x%4X \n", address, memory[address]); }
	
//			address = address % MEMORYSIZE;
			reg[rt] = lw(address, memory);
			if(flag[HIDEIND] != 1) { printf("\t\treg[rt(%u)] = 0x%X\n", rt, reg[rt]); }
		}
	} else if (opcode == SW) {
		// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm番地に書き込む
		opNum[SW]++;

		if( im >= 0x8000 ) {	//imは16bit
			im = im | 0xFFFF0000;
		}
		address = (reg[rs]+im);

		/* Memory mapped I/O対応 */
		if( (address & MMIO) == MMIO) {
			if(flag[HIDEIND] != 1) { printf("\t(im:0x%X)/(reg[%2u]:0x%X)/(reg[%2u]:0x%X)\n", im, rs, reg[rs], rt, reg[rt]); }
			writeCond = fwrite(&reg[rt], sizeof(unsigned char), 1, soFile);
			if(srOutCount < FILESIZE) {
				srOut[srOutCount] = reg[rt] & 0xFF;
				flag[OUTPUTSIZE]++;
				srOutCount++;
			}
			if(writeCond == 0) { fprintf(stderr, "failed to fwrite at \n"); }
			if(flag[HIDEIND] != 1) {
				printf("\tMMIOWRITE:\twrite to serialport (%u).\n", reg[rt]);
			}
//			im = writeCond;
		} else {
			if(address > MEMORYSIZE && (address & MMIO) != MMIO) {
				fprintf(stderr, "[ ERROR ] Memory overflow\n");
				exit(1);
			}
//			address = address % MEMORYSIZE;
			if(flag[HIDEIND] != 1) { printf("\tSW :\t[address: 0x%04X-0x%04X] <- [$%2u(rt): 0x%2X]\n", address, address+3, rt, reg[rt]); }
			if(address > MEMORYSIZE) {
				fprintf(stderr, "[ ERROR ]\tMemory overflow\n");
				exit(1);
			}
			sw(reg[rt], address, memory);
			if(flag[HIDEIND] != 1) { printf("\tmemory[address: 0x%04X-0x%04X] : %02X %02X %02X %02X\n", address, address+3, memory[address+3], memory[address+2], memory[address+1], memory[address]); }
		}
	} else if (opcode == LUI) {
		/* 
			Description: The immediate value is shifted left 16 bits and stored in the register. The lower 16 bits are zeroes.
			Operation:	$t = (imm << 16); advance_pc (4);
			Syntax:		lui $t, imm 
		*/
		reg[rt] = im << 16;
		if(flag[HIDEIND] != 1) { printf("\tLUI :\tReg[$%2u] <- [0x%4X (imm)]\n", rt, im); }
		opNum[LUI]++;
	} else if (opcode == JAL) {
		jump = instruction & 0x3FFFFFF;
		flag[JUMPFLG] = 1;
		if(flag[HIDEIND] != 1) { printf("\tJAL: (jump_to) 0x%04x ($ra <- pc[%X])\n", jump*4, pc+4); }
		reg[31] = pc+4;
		pc = jump*4 + PCINIT;
		opNum[JAL]++;
		labelRec[pc]++;
//		printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
	} else if (opcode == ORI) {
		opNum[OR]++;
		alutemp = reg[rs];
		reg[rt] = ori(reg[rs], im);
		if(flag[HIDEIND] != 1) { printf("\t[reg($%02u):0x%4X] OR? [imm:0x%4X] => [reg($%02u)] 0x%2X\n", rs, alutemp, im, rt, reg[rt]); }
	} else if (opcode == ANDI) {
		/* R[rt] = R[rs] & ZeroExtImm */
		opNum[ANDI]++;
		reg[rt] = reg[rs] & im;
		if(flag[HIDEIND] != 1) { printf("\t[$%2u:0x%4X] ANDi? [im:0x%4X] => [$%2u] 0x%4X\n", rs, reg[rs], im, rt, reg[rt]); }
	} else {
		printf("\t[Unknown OPCODE]\n");
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

double getProcTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return( (double) tv.tv_sec + (double) tv.tv_usec * 0.000001 );
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
	unsigned long long breakpoint = 0xFFFFFFFF;
	unsigned char *input;
	unsigned char *srOut;
	unsigned int *memory;

	unsigned int reg[REGSIZE], fpreg[REGSIZE];	// 32 register
	int i = 0;
	unsigned int count = 0;
	unsigned int snum = 0;	// serial portに書き出したbyte数
	unsigned int srCount=0;
	unsigned int mstart = 0, mfinish = 0xFFFFFFFF;
	unsigned int fpuNum[OPNUM] = { 0 };	//各浮動小数点命令の実行回数 fpuNum[OPCODE]++ の形で使用
	unsigned int opNum[OPNUM];	//各命令の実行回数 opNum[OPCODE] の形で使用/function系はopNum[FUNCCODE+128]
	unsigned int labelRec[BUFF];
	char sequentialBuff[BUFF];
	FILE* soFile;
	unsigned int fSize;
	int comp;
	double timebuff[BUFF];
	int i_time = 0;

	timebuff[i_time] = getProcTime();
//	fprintf(stderr, "time[%2d] = %f\n", i_time, timebuff[i_time]);

	for(count=0;count<BUFF;count++) {
		labelRec[count]=0;
	}
	
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
	fprintf(stderr, "fileSize = %u (Bytes)\n", fSize);
	if(maxpc==0) {
		perror("[ ERROR ]\tfailed to read input file.\n");
		exit(1);
	} else {
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
		} else { flag[HIDEIND] = 0; }

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
		} else { flag[5] = 0; }
		/* printmem */
		comp = strcmp(argv[i],PRINTMEM);
		if((comp == 0) && (argc > (i+2))) { 
			flag[6] = 1;
			mstart = (unsigned int) atoi(argv[i+1]);
			mfinish = (unsigned int) atoi(argv[i+2]);
			printf("PRINTMEM, ");
		} else { flag[6] = 0; }
		/* native */
		comp = strcmp(argv[i],FPUNATIVE);
		if((comp == 0)) {
			flag[7] = 1;
			printf("FPUNATIVE, ");
		} else { flag[7] = 0; }
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
	fprintf(stderr, "time[%2d] = %.12f\n", i_time, (timebuff[i_time] - timebuff[i_time-1]) );


	i=0;
	count=0;
	flag[SDATA] = 0;
	printf("\nmaxpc:0x%X(%uline)\n", maxpc+PCINIT, (maxpc)/4);
	while(count < maxpc) {
		if (opBuff[count] == 0xFFFFFFFF) {
			flag[SDATA] = 1;
			maxpc=(count+1)*4;
			count++;
			fSize = fSize - count*4;
			break;
		}
		if ( (opBuff[count] != 0) && (flag[SDATA] == 0) ) {
			memory[4*count+3+PCINIT] = opBuff[count] & 0xFF;
			memory[4*count+2+PCINIT] = (opBuff[count] >> 8) & 0xFF;
			memory[4*count+1+PCINIT] = (opBuff[count] >> 16) & 0xFF;
			memory[4*count+PCINIT] = opBuff[count] >> 24;
		}
		if(count > MEMORYSIZE) { fprintf(stderr, "[ ERROR ]\tMemory overflow.\n"); exit(1); }
		if(count > BLOCKRAM) { fprintf(stderr, "[ INFO ]\tProgram has reached at BLOCKRAM.\n"); count++; break; }
		count++;
	}
	for(i=0; i<32; i++) {
		memory[4*count+i+PCINIT] = 0x0;
	}
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
	fprintf(stderr, "time[%2d] = %.12f\n", i_time, (timebuff[i_time] - timebuff[i_time-1]) );

	/* シミュレータ本体 */
	if(flag[HIDEIND] == 1) {
		while(pc < maxpc+PCINIT+1 || breakCount >= breakpoint) {	// unsigned int
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
					pc = decoderHide(pc, operation, memory, input, srOut, flag, reg, opNum, labelRec, soFile);
					break;	
			}
			breakCount++;
			if( (breakCount << 39) == 0) { 
				fprintf(stderr, "[ INFO ]\tprocessing instructions... @ %10llX <Fast mode>\n", breakCount);
				i_time++;
				timebuff[i_time] = getProcTime();
				fprintf(stderr, "\t\ttime[%2d] = %.8f\n", i_time, (timebuff[i_time] - timebuff[i_time-1]) );
			}
		}
	} else {
		while(pc < maxpc+PCINIT+1) {	// unsigned int
			printf("\n== next: %u ==\n", ((pc-PCINIT)/4));
			reg[0] = 0;
			operation = memory[pc] | memory[pc+1] << 8 | memory[pc+2] << 16 | memory[pc+3] << 24;
			if(flag[HIDEIND] == 1) {
				pc = decoder(pc, operation, memory, input, srOut, breakCount, flag, reg, fpreg, opNum, fpuNum, labelRec, soFile);
			} else {
				pc = decoder(pc, operation, memory, input, srOut, breakCount, flag, reg, fpreg, opNum, fpuNum, labelRec, soFile);
			}
			if(operation != 0 && flag[PRINTREGIND] != 0 && (flag[HIDEIND] != 1)) {
				printRegister(reg);	// 命令実行後のレジスタを表示する
			}
			breakCount++;
			if(pc > (maxpc+PCINIT) || breakCount >= breakpoint) {
				printf("pc = 0x%08X, maxpc = 0x%08X\n", pc, maxpc+PCINIT);
				break;
			}
			if(flag[5] == 1) { 
				if (fgets(sequentialBuff, sizeof(sequentialBuff) - 1, stdin) == NULL) { ; }
			}
			if(breakCount%10000000 == 0) { fprintf(stderr, "\tprocessing instructions... %llu\n", breakCount);}
		}
		if(flag[HIDEIND] != 0) { printf("\n[ FINISHED ]\n"); }
	}

	fprintf(stderr, "[ INFO ]\tprocessed instructions... @ %10llX <Fast mode>\n", breakCount);
	i_time++;
	timebuff[i_time] = getProcTime();
	fprintf(stderr, "\t\ttime[%2d] = %.8f\n", i_time, (timebuff[i_time] - timebuff[i_time-1]) );

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
		if (labelRec[snum] != 0 && ((count+1) % 2) == 0) {
			printf("Line: %6u -> %12u (回)\n\t",(snum-PCINIT)/4,labelRec[snum]);
			count++;
		} else if (labelRec[snum] != 0) {
			printf("Line: %6u -> %12u (回), ",(snum-PCINIT)/4,labelRec[snum]);
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
	i_time++;
	timebuff[i_time] = getProcTime();
	fprintf(stderr, "time[%2d] = %.8f\n", i_time, (timebuff[i_time] - timebuff[i_time-1]) );
	fprintf(stderr, "time = %.1f\n", (timebuff[i_time] - timebuff[0]));

	if(soFile != NULL) {
		fclose(soFile);
	}
	close(fd1);
	return 0;
}

