#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include "const.h"
#include "fpu/C/fpu.h"
#include "print.h"
#include "alu.h"




//unsigned int rZ, rV, rN, rCarry;	// condition register
//unsigned int pc;		// program counter: jump -> memory[pc]
//unsigned int opNum[128+OPNUM];	//各命令の実行回数 opNum[128+OPCODE]++ の形で使用
int jumpFlg;

unsigned int signExt(unsigned int argument) {
	if(argument > 0x10000) return argument;
	if(argument & 0x8000) {
		argument = argument | 0xFFFF8000;
	}
	return argument;
}

unsigned int fpu(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, int* flag, unsigned int* fpuNum) {
	unsigned int fpfunction = 0;
	unsigned int fmt=0;
	unsigned int ft=0;
	unsigned int rt=0;
	unsigned int fs=0;
	unsigned int fd=0;
//	unsigned int im=0;
	unsigned int itoftemp=0;
	unsigned int ftoitemp=0;
	unsigned int fputemp=0;
	unsigned int target = 0;
	/* [op:6] [fmt:5] [ft:5] [fs:5] [fd:5] [funct:6] */
	/* [op:6] [fmt:5] [ft:5] [im:16] */
	fmt = (instruction >> 21) & 0x1F;
	ft  = (instruction >> 16) & 0x1F;
	rt  = (instruction >> 16) & 0x1F;
	fs  = (instruction >> 11 ) & 0x1F;
	fd  = (instruction >> 6 ) & 0x1F;

	if(0) { pc=pc; }
/*
○ 			mfc1 				010001 00000 rt 	fs 00000 000000 	rt <- fs 	FPUレジスタ → 汎用レジスタ
○ 			mtc1 				010001 00100 rt 	fs 00000 000000 	fs <- rt 	汎用レジスタ → FPUレジスタ
○ 			mov.s 				010001 10000 00000 	fs fd 000110 	fd <- fs 	FPUレジスタ移動
○ 			add.s fd, fs, ft 	010001 10000 ft 	fs fd 000000 
*/
	fpfunction = instruction & 0x3F;
	if (fmt == BC1) {
		target = instruction & 0xFFFF;
		if (ft == 0) {		// falseで分岐
			if(flag[1] != 1)  printf("\tMOVS : $FP%02u <- $FP%02u\n", fd, fs);
			fputemp = fpreg[23] & 0x800000;
			if(fputemp == 0) {
				pc = pc + 4 + signExt(target)*4;
				if(flag[1] != 1) printf("\tBC1F : jump -> 0x%X\n", pc);
			} else {
				if(flag[1] != 1) printf("\tBC1F : NOP\n");
			}
			fpuNum[BC1F]++;
		} else if(ft == 1) {	// Trueで分岐
			fputemp = fpreg[23] & 0x800000;
			if(fputemp == 0x800000) {
				pc = pc + 4 + signExt(target)*4;
				if(flag[1] != 1) printf("\tBC1T : jump -> 0x%X\n", pc);
			} else {
				if(flag[1] != 1) printf("\tBC1T : NOP\n");
			}
			fpuNum[BC1T]++;
		} else {
			printf("\t[ ERROR ]\tUnknown BC1 option(fmt == 0 && (ft != 0 || ft != 1))\n");
		}		
	} else {
		if(instruction != 0 && flag[1] != 1) {
			printf("\t[fpfunction:%2X]\n", fpfunction);
		}
		switch (fpfunction) {
			case (0) :
				if(fmt == MFC1M) {
					reg[rt] = fpreg[fs];
					if(flag[1] != 1) printf("\tMFC1 : $%02u <- $FP%02u\n", rt, fs);
					fpuNum[FMFC]++;
				} else if (fmt == MTC1M) {
					fpreg[fs] = reg[rt];
					if(flag[1] != 1) printf("\tMTC1 : $FP%02u <- $%02u\n", fs, rt);
					fpuNum[FMTC]++;
				} else if (fmt == 0x10) {
					if (fd == fs)
						fputemp = fpreg[fs];
					else
						fputemp = fpreg[ft];
					fpreg[fd]=fadd (fpreg[fs], fpreg[ft]);
					if(flag[1] != 1) {
						if (fd == fs)
							printf("\tFADD : ($FP%02u)%X = ($FP%02u)%X + ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fputemp);
						else {
							printf("\tFADD : ($FP%02u)%X = ($FP%02u)%X + ($FP%02u)%X\n", fd, fpreg[fd], ft, fputemp, fs, fpreg[fs]);
						}
					}
					fpuNum[FADDS]++;
				} else {
					printf("Unknown fmt(function '0').\n");
				}
				break;
			case (MOVSF) :	
				if(fmt == 0x10) { 
					fpreg[fd] = fpreg[fs];
					if(flag[1] != 1)  printf("\tMOVS : $FP%02u <- $FP%02u\n", fd, fs);
					fpuNum[MOVSF]++;
				}
				break;
			case (SQRT) :
				/* fd <- sqrt(fs) 	fsqrt  */
				if(fmt == 0x10) { 
					fpreg[fd] = fsqrt(fpreg[fs]);
					if(flag[1] != 1) printf("\tSQRT : ($FP%02u)%X <- SQRT(%02u:%X)\n", fd, fpreg[fd], fs, fpreg[fs]);
					fpuNum[SQRT]++;
				}
				break;
			case (FSUBS) :
				if(fmt == 0x10) { 
					if (fd == fs) {
						fputemp = fpreg[fs];
					} else {
						fputemp = fpreg[ft];
					}
					fpreg[fd]=fsub (fpreg[fs], fpreg[ft]);
					if(flag[1] != 1) {
						if (fd == fs)
							printf("\tFSUB : ($FP%02u)%X = ($FP%02u)%X - ($FP%02u)%X\n", fd, fpreg[fd], fs, fputemp, ft, fpreg[ft]);
						else
							printf("\tFSUB : ($FP%02u)%X = ($FP%02u)%X - ($FP%02u)%X\n", fd, fpreg[fd], fs, fpreg[fs], ft, fputemp);
					}
					fpuNum[FSUBS]++;
				}
				break;
			case (FMULS) :	
				if(fmt == 0x10) { 
					fpreg[fd]=fmul (fpreg[fs], fpreg[ft]);
					if(flag[1] != 1) printf("\tFMUL : ($FP%02u)%X = ($FP%02u)%X * ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fpreg[fs]);
					fpuNum[FMULS]++;
				}
				break;
			case (FDIVS) :	
				if(fmt == 0x10) { 
					if (fd == fs)
						fputemp = fpreg[fs];
					else
						fputemp = fpreg[ft];
					fpreg[fd]=fdiv (fpreg[fs], fpreg[ft]);
					if (fd == fs)
						printf("\tFDIV : ($FP%02u)%X = ($FP%02u)%X / ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fputemp);
					else
						printf("\tFDIV : ($FP%02u)%X = ($FP%02u)%X / ($FP%02u)%X\n", fd, fpreg[fd], ft, fputemp, fs, fpreg[fs]);
					fpuNum[FDIVS]++;
				}
				break;
			case (FTOIF) :
				if(fmt == FTOIM) {
					ftoitemp = fpreg[fs];
					fpreg[fd]=ftoi (fpreg[fs]);
					if(flag[1] != 1) printf("\tFTOI :($FP%02u)%X -> ($FP%02u)%X\n", fs, ftoitemp, fd, fpreg[fd]);
					fpuNum[FTOI]++;
				}
				break;
			case (ITOFF) :
				if(fmt == ITOFM) {
					itoftemp = fpreg[fs];
					fpreg[fd]=itof (fpreg[fs]);
					if(flag[1] != 1) printf("\tITOF : ($FP%02u)%X -> ($FP%02u)%X\n", fs, itoftemp, fd, fpreg[fd]);
					fpuNum[ITOF]++;
				}
				break;
			case (CEQ) :
				if(fmt == 0x10) {
					if(feq (fpreg[fs], fpreg[ft])) {
						fpreg[23] = fpreg[23] | 0x800000;
					} else {
						fpreg[23] = fpreg[23] & 0xFF7FFFFF;
					}
					if(flag[1] != 1) printf("\tC.EQ : ?(($FP%02u)%X == ($FP%02u)%X) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]);
					fpuNum[CEQ]++;
				}
				break;
			case (COLT) :
				if(fmt == 0x10) {
					if(flt (fpreg[fs], fpreg[ft])) {
						fpreg[23] = fpreg[23] | 0x800000;
					} else {
						fpreg[23] = fpreg[23] & 0xFF7FFFFF;
					}
					if(flag[1] != 1) printf("\tC.OLT : ?(($FP%02u)%X < ($FP%02u)%X) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]);
					fpuNum[COLT]++;
				}
				break;
			case (COLE) :
				if(fmt == 0x10) {
					if(fle (fpreg[fs], fpreg[ft])) {
						fpreg[23] = fpreg[23] | 0x800000;
					} else {
						fpreg[23] = fpreg[23] & 0xFF7FFFFF;
					}
					if(flag[1] != 1) printf("\tC.OLE : ?(($FP%02u)%X <= ($FP%02u)%X) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]);
					fpuNum[COLE]++;
				}
				break;
			default :
				printf("Unknown FPswitch has selected.\n");
		}
	}
	if(flag[1] != 1 || flag[3] != 1) printFPRegister(fpreg);
	return pc;
}

/*
RRB (RS-232C Receive Byte)
31 - 26 = 011100
25 - 21 = 00000
20 - 16 : rt
15 -  0 : 0000000000000000 RS-232Cを通じて1バイト受信し、0拡張してレジスタrt内に保存する。
FIFOが空の場合、ブロッキングする。
RSB (RS-232C Send Byte)
31 - 26 = 011101
25 - 21 = 00000
20 - 16 : rt
15 -  0 : 0000000000000000 レジスタrt内の下位8ビットをRS-232Cを通じて送信する。
FIFOが一杯の場合、ブロッキングする。 
*/
unsigned int rrb(unsigned char* serialin) {
	static unsigned int numin = 0;
	unsigned int rt;
	rt = (unsigned int) serialin[numin];
	printf("\t[ DEBUG ]\tSERIALIN rt[%u]:%02X(%02X)\n", numin, rt, serialin[numin]);
	numin++;
	return rt;
}

/* Send byte to serial port */
unsigned int rsb(unsigned int rt, unsigned char* serial) {
	static unsigned int numout = 0;
	serial[numout] = (unsigned char) (rt & 0xFF);
	printf("\t[ DEBUG ]\tSERIALOUT(%04u) rt(%02X):%02X\n", numout, rt, serial[numout]);
	numout++;
	return 0;
}

/* store word */
// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm(符号拡張)番地に書き込む ☆sw()呼び出し時点で拡張済み
/* リトルエンディアン */
unsigned int sw(unsigned int rt, unsigned int address, unsigned int* memory, unsigned int stackPointer) {
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int addr3;

	addr3 = (rt & 0xFF000000) >> 24;
	addr2 = (rt & 0x00FF0000) >> 16;
	addr1 = (rt & 0x0000FF00) >>  8;
	addr0 = rt & 0x000000FF;

	if (stackPointer == 31) {
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
	if(rt != ( memory[address] | memory[address+1] << 8 | memory[address+2] << 16 | memory[address+3] << 24 )) {
		printf("\nFailed to SW\n");
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


/* opcodeが0の時の操作を、末尾6ビットによって決める */
unsigned int funct (unsigned int pc, unsigned int instruction, int* flag, unsigned int* reg, unsigned int* opNum) {
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

	if(instruction != 0 && flag[1] != 1) {
		printf("\t[OPERAND] rs:%08X rt:%08X rd:%08X shamt:%08X \n", rs,rt,rd,shamt);
	}
	
	function = instruction & 0x3F;
	if(instruction != 0) printf("\t[function:%2X]\n", function);
	switch (function) {
		case (JR) :
			pc = reg[rs];
			if(flag[1] != 1) printf("\tJR : $ra(%u) = 0x%X / pc -> 0x%X\n", rs, reg[rs], pc);
			jumpFlg = 1;
			opNum[128+JR]++;
			break;
		case (ADDU) :	// rd=rs+rt
			rs_original = reg[rs];
			reg[rd] = addu(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\tADDU :\t[$%2u: 0x%2X] + [$%2u: 0x%2X] => [$%2u: 0x%2X]\n", rs, rs_original, rt, reg[rt], rd, reg[rd]);
			opNum[128+ADDU]++;
			break;
		case (SUBU) :	// rd=rs-rt
			rs_original = reg[rs];
			reg[rd] = subu(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\tSUBU :\t[$%2u 0x%2X], [$%2u 0x%2X] => [rd:%u] 0x%2X\n", rs, rs_original, rt, reg[rt], rd, reg[rd]);
			opNum[128+SUBU]++;
			break;
		case (SLT) :
			if(flag[1] != 1) printf("\tSLT :\t?([$%2u: 0x%04x] < [$%2u: 0x%04x]) ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				if(flag[1] != 1) printf("\t=> [$%2u: 0x%4X] <TRUE>\n", rd, reg[rd]);
			} else {
				if(flag[1] != 1) printf("\t=> [$%2u: 0x%4X] <FALSE>\n", rd, reg[rd]);
			}
			opNum[128+SLT]++;
			break;
		case (SLL) :
			if(rs == 0 && shamt == 0) {
				if(flag[1] != 1) printf("\tNOP\n");
				opNum[128+NOP]++;
				break;
			}
			reg[rd] = sll(reg[rs], shamt);
			if(flag[1] != 1) printf("\tSLL :\t%u << %u -> %u\n", reg[rs], shamt, reg[rd]);
			opNum[128+SLL]++;
			break;
		case (SRL) :
			if(rs == 0 && shamt == 0)
				printf("\tNOP\n");
				opNum[128+NOP]++;
				break;
			reg[rd] = srl(reg[rs], shamt);
			if(flag[1] != 1) printf("\tSRL :\t%u << %u -> %u\n", reg[rs], shamt, reg[rd]);
			opNum[128+SRL]++;
			break;
		case (AND) :
			reg[rd] = and(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\t[$%2u:0x%4X] AND? [$%2u:0x%4X] => [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			opNum[128+AND]++;
			break;
		case (OR) :
			reg[rd] = or(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\t[$%2u:0x%4X] OR? [$%2u:0x%4X] => [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			opNum[128+OR]++;
			break;
		default :
			printf("Unknown switch has selected.\n");
	}
	return pc;
}
/* デコーダ */
unsigned int decoder (unsigned int pc, unsigned int instruction, unsigned int* memory,unsigned char* serialin, unsigned char* serial, unsigned long long breakCount, int* flag, unsigned int* reg, unsigned int* fpreg, unsigned int* opNum, unsigned int* fpuNum) {
	unsigned int opcode=0;
	unsigned int rt=0;
	unsigned int rs=0;
	unsigned int rs_original;
	unsigned int im=0;
	unsigned int jump=0;
	unsigned int line=0;
	unsigned int address=0;
	unsigned int alutemp=0;

	opcode = instruction >> 26;	// opcode: 6bitの整数
	rs = (instruction >> 21) & 0x1F;
	rt = (instruction >> 16) & 0x1F;
	im = instruction & 0xFFFF;

	if(instruction != 0 && flag[1] != 1) {
		printf("[ops: %06llu,pc: 0x%x]\n", breakCount, pc);
		printf("[instruction: 0x%2X]\n", instruction);
		printf("\t[opcode:%2X]\n", opcode);
	}

	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) {
		pc = funct(pc, instruction, flag, reg, opNum);
	} else if (opcode == FPU) {
		if(flag[1] != 1) printf("\tFPU \n");
		pc = fpu(pc, instruction, reg, fpreg, flag, fpuNum);
	} else if (opcode == SRCV) {
		reg[rt] = rrb(serialin);
		if(flag[1] != 1) printf("\tSRCV(rrb):\t reg[%u]:%02X  \n",rt,reg[rt]);
		opNum[SRCV]++;
	} else if (opcode == SSND) {
		if(flag[1] != 1) printf("\tSSND(rsb):\t \n");
		rsb(reg[rt], serial);
		opNum[SSND]++;
	} else if (opcode == ADDIU) {
		if( im >= 0x8000 ) {
			im = im | 0xFFFF0000;
//			printf("(%X)",Imm);
		}
		rs_original = reg[rs];
		reg[rt] = (addiu(reg[rs], im, rt) & 0xFFFFFFFF);	// 処理部
		if(flag[1] != 1) printf("\tADDIU :\t[$%2u: 0x%4X] + [im: 0x%8X] => [$%2u: 0x%4X]\n", rs, rs_original, im, rt, reg[rt]);
		opNum[ADDIU]++;
	} else if (opcode == JUMP) {
		jumpFlg = 1;
		jump = instruction & 0x3FFFFFF;
		if(flag[1] != 1) printf("\tJ :\t(jump_to) 0x%04x\n", jump*4);
		pc = jump*4 + PCINIT;	// jumpはpAddr形式
		opNum[JUMP]++;
	} else if (opcode == BEQ) { // beq I-Type: 000100 rs rt BranchAddr 	等しいなら分岐 
		if(flag[1] != 1) printf("\tBEQ :\t?([$%2u 0x%2X]=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + line*4);
		if(reg[rs] == reg[rt]) {
			jumpFlg = 1;
			// 0x48 = 0x20 + 4 + line*4 +0x0	<-> line*4 = 0x48-0x24 <-> line = 9
			pc = pc + 4 + line*4;		// pAddr形式
			if(flag[1] != 1) printf("\t\t<TRUE & JUMP> -> (branch_to) 0x%04x\n", pc);
		} else {
			if(flag[1] != 1) printf("\t<FALSE & NOP>\n");
		}
		opNum[BEQ]++;
	} else if (opcode == BNE) { // bne I-Type: 000100 rs rt BranchAddr 	等しくないなら分岐 
		if(flag[1] != 1) printf("\tBNE :\t?([$%2u 0x%2X]!=[$%2u 0x%2X]) -> branch(from 0x%04x to 0x%04x)\n", rs, reg[rs], rt, reg[rt], pc, pc + 4 + line*4);
		if(reg[rs] != reg[rt]) {
			jumpFlg = 1;
			pc = pc + 4 + line*4;		// pAddr形式
			if(flag[1] != 1) printf("\t\t<TRUE & JUMP> -> (branch_to) 0x%04x\n", pc);
		} else {
			if(flag[1] != 1) printf("\t<FALSE & NOP>\n");
		}
		opNum[BNE]++;
	} else if (opcode == LW) {	// 0x47: lw r1, 0xaaaa(r2) : r2+0xaaaaのアドレスにr1を32ビットでロード

		/* じかんがあるときにlw()内に移動する */
		if( im >= 0x8000 ) {	//imは16bit
			im = im & 0x00007FFF;
			if(flag[1] != 1) printf("(im:0x%X)", im);
			address = (reg[rs] + MEMORYSIZE - im);
		} else {
			address = (reg[rs]+im);
		}
		if(address > MEMORYSIZE) {
			printf("[] Memory overflow\n");
			exit(1);
		}
		address = address % MEMORYSIZE;
		if(flag[1] != 1) printf("\tLW :\tmemory[address:0x%04X]=0x%4X \n", address, memory[address]);
		reg[rt] = lw(address, memory);
		opNum[LW]++;
		if(flag[1] != 1) printf("\t\treg[rt(%u)] = 0x%X\n", rt, reg[rt]);
	} else if (opcode == SW) {
		// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm番地に書き込む

		if(reg[rs] > MEMORYSIZE) {	/* rsがメモリサイズを越えているかどうかで分岐:この実装大丈夫か？ */
			if(flag[1] != 1) printf("(im:0x%X)/(reg[rs]:0x%X)\n", im, reg[rs]);
			if( im >= 0x8000 ) {	//符号拡張
				im = im & 0x00007FFF;
				address = (reg[rs]%MEMORYSIZE - im);
			} else {
				address = (reg[rs]%MEMORYSIZE + im);
			}
		} else {
			if( im >= 0x8000 ) {	//符号拡張
				im = im & 0x00007FFF;
				address = (reg[rs] - im);
			} else {
				address = (reg[rs] + im);
			}
		}
		address = address % MEMORYSIZE;
		if(flag[1] != 1) printf("\tSW :\t[address: 0x%04X-0x%04X] <- [$%2u(rt): 0x%2X]\n", address, address+3, rt, reg[rt]);
		if(address > MEMORYSIZE) {
			printf("[] Memory overflow\n");
			exit(1);
		}
		sw(reg[rt], address, memory, rs);
		if(flag[1] != 1) printf("\tmemory[address: 0x%04X-0x%04X] : %02X %02X %02X %02X\n", address, address+3, memory[address+3], memory[address+2], memory[address+1], memory[address]);
		opNum[SW]++;
	} else if (opcode == LUI) {
		/* 
			Description: The immediate value is shifted left 16 bits and stored in the register. The lower 16 bits are zeroes.
			Operation:	$t = (imm << 16); advance_pc (4);
			Syntax:		lui $t, imm 
		*/
		reg[rt] = im << 16;
		if(flag[1] != 1) printf("\tLUI :\tReg[0x%02X] <- [0x%4X (imm)]\n", rt, im);
		opNum[LUI]++;
	} else if (opcode == JAL) {
		jump = instruction & 0x3FFFFFF;
		jumpFlg = 1;
		if(flag[1] != 1) printf("\tJAL: 0x%04x ($ra <- pc[%X])\n", jump*4, pc+4);
		reg[31] = pc+4;
		pc = jump*4 + PCINIT;
		opNum[JAL]++;
	} else if (opcode == ORI) {
		alutemp = reg[rs];
		reg[rt] = ori(reg[rs], im);
		if(flag[1] != 1) printf("\t[reg($%02u):0x%4X] OR? [imm:0x%4X] => [reg(%02u)] 0x%2X\n", rs, alutemp, im, rt, reg[rt]);
		opNum[OR]++;
	} else {
		printf("[Unknown OPCODE]\n");
	}

	if(jumpFlg == 0) {
		pc = pc + 4;
		jumpFlg = 0;
	} else {
		jumpFlg = 0;
	}
	return pc;
}

int main (int argc, char* argv[]) {
	unsigned int opBuff[BUFF];	// ファイルから読み込む命令列
//	int srRead=0;
	int fd1 = 0, fd2 = 0;
	unsigned int maxpc;
	unsigned long long breakCount = 0;
	unsigned long long breakpoint = 0x800000;
	unsigned int pc = 0;
	unsigned int *memory;
	unsigned char *serial;
	unsigned char *serialin;
	unsigned int operation;	// 実行中命令

	unsigned int reg[REGSIZE];	// 32 register
	unsigned int fpreg[REGSIZE];	// 32 fpregister
	int i=2;
	unsigned int pAddr = 0;
	unsigned int count = 0;
	unsigned int snum = 0;	// serial portに書き出したbyte数
	int flag[REGSIZE];
	unsigned int mstart = 0, mfinish = 0xFFFFFFFF;
	unsigned int fpuNum[OPNUM] = { 0 };	//各浮動小数点命令の実行回数 fpuNum[OPCODE]++ の形で使用
	unsigned int srCount=0;
	unsigned int srLine = 0;
	unsigned int opNum[OPNUM];	//各命令の実行回数 opNum[OPCODE] の形で使用/function系はopNum[FUNCCODE+128]

//	int orderNum;
	/* flag[0]:help flag[1]:hide flag[5]:sequential  */
	for(count=0;count<BUFF;count++) {
		opBuff[count]=0;
	}
	
	count=0;
//	printf("\n\t====== Initialize ======\n");
	memory = (unsigned int *) calloc( MEMORYSIZE, sizeof(unsigned int) );
	if(memory == NULL) {
		perror("memory allocation error\n");
		return -1;
	}
	serial = (unsigned char *) calloc( MEMORYSIZE, sizeof(unsigned char) );
	if(serial == NULL) {
		perror("memory allocation error\n");
		return -1;
	}
	serialin = (unsigned char *) calloc( MEMORYSIZE, sizeof(unsigned char) );
	if(serialin == NULL) {
		perror("memory allocation error\n");
		return -1;
	}

	/* 引数として<ファイル:メモリ>をとる。なければ強制終了 */
	if (argc < 2) {
		printhelp();
		return -1;
		printf("please input file.\n");
	}
	/* ファイルから実行命令列を読み込む */
	printf("%s\n", argv[1]);
	fd1 = open(argv[1], O_RDONLY);
	maxpc = read(fd1, opBuff, BUFF);
	if(maxpc==0) {
		perror("opBuff failed to read\n");
		return -1;
	} else {
//		printf("opBuff succeeded to read\n");
	}


	/* 引数の処理 */
	while(i < argc) {
		/* help */
		flag[0] = strcmp(argv[i], HELP);
		if(flag[0] == 0) { printhelp(); exit(1); }
		/* hide */
		flag[1] = strcmp(argv[i], HIDE);
		if(flag[1] == 0) { flag[1] = 1; } else { flag[1] = 0; }
		/* serialin */
		flag[2] = strcmp(argv[i], SERIALIN);
		if(flag[2] == 0) { flag[2] = i; printf("%s\n", argv[flag[2]+1]); } else { flag[2] = 0; }
		/* printreg */
		flag[3] = -1;
		flag[3] = strcmp(argv[i], PRINTREG);
		if(flag[3] == 0) { flag[3] = 1; }
		/* breakpoint */
		flag[4] = strcmp(argv[i], BREAKPOINT);
		if(flag[4] == 0 && argc >= i+1) { breakpoint = (unsigned long long) atoi(argv[i+1]); printf("breakpoint = %llu\n", breakpoint); }
		/* sequential */
		flag[5] = strcmp(argv[i],SEQUENTIAL);
		if(flag[5] == 0) { flag[5] = 1; } else { flag[5] = 0; }
		/* printmem */
		flag[6] = strcmp(argv[i],PRINTMEM);
		if((flag[6] == 0) && (argc > (i+2))) { 
			flag[6] = 1;
			mstart = (unsigned int) atoi(argv[i+1]);
			mfinish = (unsigned int) atoi(argv[i+2]); 
		} else { flag[6] = 0; }
		/* native */
		flag[7] = strcmp(argv[i],FPUNATIVE);
		if((flag[7] == 0)) { flag[7] = 1; } else { flag[7] = 0; }
		/* serialout */
		flag[8] = strcmp(argv[i],SERIALOUT);
		if((flag[8] == 0) && (argc > (i+2))) { flag[8] = 1; } else { flag[8] = 0; }
		i++;
	}
	i=0;

	count=0;
/*	if(flag[1] != 1) {
		while(count<40) {
			if(opBuff[count] != 0) printf("%02u(pc:%2X):\t%8X\n", count, (count)*4, opBuff[count]);
			count++;
		}
	}
*/
	/* 入力文字列をmemoryのPCINIT番地以降にコピーする。0xFFFFFFFFがきたら処理を切り替える */
	count = 0;
	flag[31] = 0;
	printf("maxpc:0x%X(%uline)\n\n", maxpc+PCINIT, (maxpc)/4);
	while(count < maxpc) {
//		memory[count*4+PCINIT] = opBuff[count];
		if ( (opBuff[count] != 0) && (flag[31] == 0) ) {
			memory[4*count+3+PCINIT] = opBuff[count] & 0xFF;
			memory[4*count+2+PCINIT] = (opBuff[count] >> 8) & 0xFF;
			memory[4*count+1+PCINIT] = (opBuff[count] >> 16) & 0xFF;
			memory[4*count+PCINIT] = opBuff[count] >> 24;
//			printf("[ DEBUG ]\t%08X, %u\n", opBuff[count], count);
//			printf("[ DEBUG ]\t%02X%02X%02X%02X\n", memory[4*count+PCINIT+3], memory[4*count+PCINIT+2], memory[4*count+PCINIT+1], memory[4*count+PCINIT]);
		}
		if (flag[31] == 0 && memory[4*count+PCINIT+3]==0xFF && memory[4*count+PCINIT+2]==0xFF && memory[4*count+PCINIT+1]==0xFF && memory[4*count+PCINIT] == 0xFF) {
			flag[31] = 1;
			memory[4*count+3+PCINIT] = 0;
			memory[4*count+2+PCINIT] = 0;
			memory[4*count+1+PCINIT] = 0;
			memory[4*count+PCINIT] = 0;
			count++;
			maxpc=(count+1)*4;
			printf("[ DEBUG ]\t(%u)Serial in.\n\n",count);
			break;
		}
		if(count > MEMORYSIZE) { printf("[ ERROR ]\tMemory overflow.\n"); }
		count++;
	}
	if (flag[31] == 1) {
		while(count < BUFF) {
			/* データ相当分 */

			serialin[4*srCount+3] = opBuff[count] & 0xFF;
			serialin[4*srCount+2] = (opBuff[count] >> 8) & 0xFF;
			serialin[4*srCount+1] = (opBuff[count] >> 16) & 0xFF;
			serialin[4*srCount] = opBuff[count] >> 24;

			if(opBuff[count] != 0) {
				printf("SP%04u(%08X): %02X %02X %02X %02X, ", srCount*4, opBuff[count], serialin[4*srCount+3], serialin[4*srCount+2], serialin[4*srCount+1], serialin[4*srCount]);
				srLine++;
			}
			srCount++;
			if(srLine % 4 == 0 && opBuff[count] != 0) {
				printf("\n");
			}
		count++;
		}
	}

	printf("\n");


	srCount=0;
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
	pAddr = pc;

	
	/* シミュレータ本体 */
	while(pc > PCINIT-1) {	// unsigned int
		if(flag[1] != 1) printf("\n====== next: %u ======\n", ((pc-PCINIT)/4));
		reg[0] = 0;
		operation = memory[pAddr] | memory[pAddr+1] << 8 | memory[pAddr+2] << 16 | memory[pAddr+3] << 24;
		pc = decoder(pc, operation, memory, serialin, serial, breakCount, flag, reg, fpreg, opNum, fpuNum);
		if(operation != 0 && flag[3] != 1) {
			if(flag[1] != 1)  printRegister(reg);	// 命令実行後のレジスタを表示する
		}
		pAddr = pc;
		if(pc > PCINIT*4) pc = PCINIT;
		breakCount++;
		if (breakCount > breakpoint) break;
		if(pc > (maxpc+PCINIT)) {
			printf("pc = 0x%08X, maxpc = 0x%08X\n", pc, maxpc+PCINIT);
			break;
		}
//		if(flag[5] == 1) { ;}
//			fgets();
	}
	if(flag[1] != 0) printf("\n[ FINISHED ]\n");
	snum = mstart;

	
	if ((mfinish - mstart) > 0) {
		printf("\nメモリダンプ\n");
		while(snum%4 != 0) {
			snum++;
		}
	}
	while((snum < MEMORYSIZE-3) && (snum < mfinish)) {
//		if(snum >= PCINIT && snum < PCINIT*0x2) {
//			snum++;
//			continue;
//		}
		if ( (memory[snum] != 0 || memory[snum+1] != 0 || memory[snum+2] != 0 || memory[snum+3] != 0) ) {
			printf("memory[0x%06X] = %02X %02X %02X %02X\n", snum, memory[snum+3], memory[snum+2], memory[snum+1], memory[snum]);
		}
		snum = snum + 4;
	}
	snum = 0;
	printf("\nレジスタ吐き出し\n");
	printRegister(reg);

	printf("\n\n");
	printf("Total instructions: \n\t%llu\n", breakCount);
	printf("Total instructions (except NOP): \n\t%llu\n", breakCount - opNum[128+NOP]);
	printf("\n(OP)\t: \t(Num), \t(Ratio)\n");
	if(opNum[128+NOP] != 0) printf("NOP 	: %6u, %05.2f (%%)\n", opNum[128+NOP], (double) 100*opNum[ADDIU]/breakCount);
//	breakCount = breakCount - opNum[128+NOP];
/* ops & function */
	printf("ADDU	: %6u, %05.2f (%%)\n", opNum[128+ADDU], (double) 100*opNum[128+ADDU]/breakCount);
	printf("ADDIU	: %6u, %05.2f (%%)\n", opNum[ADDIU], (double) 100*opNum[ADDIU]/breakCount);
	printf("SUBU	: %6u, %05.2f (%%)\n", opNum[128+SUBU], (double) 100*opNum[128+SUBU]/breakCount);
	printf("AND 	: %6u, %05.2f (%%)\n", opNum[128+AND], (double) 100*opNum[128+AND]/breakCount);
	printf("OR  	: %6u, %05.2f (%%)\n", opNum[128+OR], (double) 100*opNum[128+OR]/breakCount);
	printf("SLL 	: %6u, %05.2f (%%)\n", opNum[128+SLL], (double) 100*opNum[128+SLL]/breakCount);
	printf("SRL 	: %6u, %05.2f (%%)\n", opNum[128+SRL], (double) 100*opNum[128+SRL]/breakCount);
	printf("LW  	: %6u, %05.2f (%%)\n", opNum[LW], (double) 100*opNum[LW]/breakCount);
	printf("SW  	: %6u, %05.2f (%%)\n", opNum[SW], (double) 100*opNum[SW]/breakCount);
	printf("JUMP	: %6u, %05.2f (%%)\n", opNum[JUMP], (double) 100*opNum[JUMP]/breakCount);
	printf("JAL 	: %6u, %05.2f (%%)\n", opNum[JAL], (double) 100*opNum[JAL]/breakCount);
	printf("JR  	: %6u, %05.2f (%%)\n", opNum[128+JR], (double) 100*opNum[128+JR]/breakCount);
	printf("BEQ 	: %6u, %05.2f (%%)\n", opNum[BEQ], (double) 100*opNum[BEQ]/breakCount);
	printf("BNE 	: %6u, %05.2f (%%)\n", opNum[BNE], (double) 100*opNum[BNE]/breakCount);
	printf("SLT 	: %6u, %05.2f (%%)\n", opNum[128+SLT], (double) 100*opNum[128+SLT]/breakCount);
	printf("RRB 	: %6u, %05.2f (%%)\n", opNum[SRCV], (double) 100*opNum[SRCV]/breakCount);
	printf("RSB 	: %6u, %05.2f (%%)\n", opNum[SSND], (double) 100*opNum[SSND]/breakCount);
	printf("\n");

/* fpu */
	printf("BC1F	: %6u, %05.2f (%%)\n", fpuNum[BC1F], (double) 100*fpuNum[BC1F]/breakCount);
	printf("BC1T	: %6u, %05.2f (%%)\n", fpuNum[BC1T], (double) 100*fpuNum[BC1T]/breakCount);
	printf("FMFC	: %6u, %05.2f (%%)\n", fpuNum[FMFC], (double) 100*fpuNum[FMFC]/breakCount);
	printf("FMTC	: %6u, %05.2f (%%)\n", fpuNum[FMTC], (double) 100*fpuNum[FMTC]/breakCount);
	printf("MOVSF	: %6u, %05.2f (%%)\n", fpuNum[MOVSF], (double) 100*fpuNum[MOVSF]/breakCount);
	printf("FADD.S	: %6u, %05.2f (%%)\n", fpuNum[FADDS], (double) 100*fpuNum[FADDS]/breakCount);
	printf("FSUB.S	: %6u, %05.2f (%%)\n", fpuNum[FSUBS], (double) 100*fpuNum[FSUBS]/breakCount);
	printf("FMUL.S	: %6u, %05.2f (%%)\n", fpuNum[FMULS], (double) 100*fpuNum[FMULS]/breakCount);
	printf("FDIV.S	: %6u, %05.2f (%%)\n", fpuNum[FDIVS], (double) 100*fpuNum[FDIVS]/breakCount);
	printf("CEQ 	: %6u, %05.2f (%%)\n", fpuNum[CEQ], (double) 100*fpuNum[CEQ]/breakCount);
	printf("COLT	: %6u, %05.2f (%%)\n", fpuNum[COLT], (double) 100*fpuNum[COLT]/breakCount);
	printf("COLE	: %6u, %05.2f (%%)\n", fpuNum[COLE], (double) 100*fpuNum[COLE]/breakCount);
	printf("FTOI	: %6u, %05.2f (%%)\n", fpuNum[MOVSF], (double) 100*fpuNum[MOVSF]/breakCount);
	printf("ITOF	: %6u, %05.2f (%%)\n", fpuNum[MOVSF], (double) 100*fpuNum[MOVSF]/breakCount);
	printf("FSQRT	: %6u, %05.2f (%%)\n", fpuNum[SQRT], (double) 100*fpuNum[SQRT]/breakCount);

/*
#define FTOIF 0x24	// Function
#define FTOIM 0x10	// fMt
#define ITOFF 0x20	// Function
#define ITOFM 0x14	// fMt
*/


	snum = 0;
	printf("\nシリアルポート出力\n\t");
	printf("\t(%04u):\t",snum);
	while(snum<BUFF) {
		if (serial[snum] != 0) {
			printf("%02X ", serial[snum]);
			if( (srCount+1) % 8 == 0) { printf(" "); }
			if( (srCount+1) % 32 == 0) { printf("\n\t(%04u):\t",snum); }
			srCount++;
		}
		snum++;
	}
//	if(sInFlag == 0) { printf(":無し\n"); }
	printf("\n\n");
	free(memory);
	memory = NULL;
	free(serial);
	serial = NULL;

	close(fd1);
	close(fd2);
	return 0;
}

