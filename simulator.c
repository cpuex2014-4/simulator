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
unsigned int opNum[128];	//各命令の実行回数 opNum[OPCODE]++ の形で使用
unsigned int fopNum[128];	//各浮動小数点命令の実行回数 fopNum[OPCODE]++ の形で使用
unsigned int funcNum[128];	//各命令の実行回数 funcNum[OPCODE]++ の形で使用
int printreg=-1;
int jumpFlg;

/*
RRB (RS-232C Receive Byte)
31 - 26 = 011100
25 - 21 = 00000
20 - 16 : rt
15 -  0 : 0000000000000000RS-232Cを通じて1バイト受信し、0拡張してレジスタrt内に保存する。
FIFOが空の場合、ブロッキングする。
RSB (RS-232C Send Byte)
31 - 26 = 011101
25 - 21 = 00000
20 - 16 : rt
15 -  0 : 0000000000000000レジスタrt内の下位8ビットをRS-232Cを通じて送信する。
FIFOが一杯の場合、ブロッキングする。 
*/
/* Receive byte from serial port */


unsigned int fpu(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg) {
	unsigned int fpfunction = 0;
	unsigned int fmt=0;
	unsigned int ft=0;
	unsigned int rt=0;
	unsigned int fs=0;
	unsigned int fd=0;
	unsigned int im=0;
	unsigned int itoftemp=0;
	unsigned int ftoitemp=0;
	/* [op:6] [fmt:5] [ft:5] [fs:5] [fd:5] [funct:6] */
	/* [op:6] [fmt:5] [ft:5] [im:16] */
	/* 先頭6bitは0であることが保証済み */
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
	printf("\t[fpfunction:%2X]\n", fpfunction);

	switch (fpfunction) {
		case (0) :
			if(fmt == MFC1M) {
				reg[rt] = fpreg[fs];
				printf("\tMFC1 :\n");
			} else if (fmt == MTC1M) {
				fpreg[fs] = reg[rt];
				printf("\tMTC1 :\n");
			} else if (fmt == ADDSM) {
				fpreg[fd]=fadd (fpreg[fs], fpreg[ft]);
				printf("\tFADD : (%02u)%X = (%02u)%X + (%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fpreg[fs]);
			} else {
				printf("Unknown fmt(function '0').\n");
			}
			break;
		case (MOVSF) :	
			if(fmt == MOVSM) { 
				fpreg[fd] = fpreg[fs];
				printf("\tMOVS :\n");
			}
			break;
		case (FSUB) :	
			if(fmt == 0x10) { 
//				fpreg[fd]=fsub(fpreg[fs], fpreg[ft]);
				printf("\tFSUB :\n");
			}
			break;
		case (FMUL) :	
			if(fmt == 0x10) { 
				fpreg[fd]=fmul (fpreg[fs], fpreg[ft]);
				printf("\tFMUL : (%02u)%X = (%02u)%X + (%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fpreg[fs]);
			}
			break;
		case (FDIV) :	
			if(fmt == 0x10) { 
//				fpreg[fd]=fdiv (fpreg[fs], fpreg[ft]);
				printf("\tFDIV :\n");
			}
			break;
		case (FTOIF) :
			if(fmt == FTOIM) {
				ftoitemp = fpreg[fs];
				fpreg[fd]=ftoi (fpreg[fs]);
				printf("\tFTOI :(fp%02u)%X -> (fp%02u)%X\n", fs, ftoitemp, fd, fpreg[fd]);
			}
			break;
		case (ITOFF) :
			if(fmt == ITOFM) {
				itoftemp = fpreg[fs];
				fpreg[fd]=itof (fpreg[fs]);
				printf("\tITOF : (fp%02u)%X -> (fp%02u)%X\n", fs, itoftemp, fd, fpreg[fd]);
			}
			break;


		default :
			printf("Default FPswitch has selected.\n");
	}
	printFPRegister(reg);
	return 0;
}

unsigned int rrb(unsigned char* srBuff) {
	static unsigned int numin = 0;
	/* [op] [0-0] [rt] [^0] */
	/* 先頭&末尾6bitは0であることが保証済み */
	unsigned int rt;
	
	rt = (unsigned int) srBuff[numin];

	return rt;
}

/* Send byte to serial port */
unsigned int rsb(unsigned int rt, unsigned char* serial) {
	static unsigned int numout = 0;

	serial[numout] = (unsigned char) (rt & 0xFF);

	return 0;
}

/* store word */
// sw rs,rt,Imm => M[ R(rs)+Imm ] <- R(rt)	rtの内容をメモリのR(rs)+Imm(符号拡張)番地に書き込む
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
unsigned int funct (unsigned int pc, unsigned int instruction, int* flag, unsigned int* reg) {
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
	
	function = instruction & 0x3F;
	if(instruction != 0) printf("\t[function:%2X]\n", function);
	switch (function) {
		case (JR) :
			pc = reg[rs];
			if(flag[1] != 1) printf("\tJR : $ra(%u) = 0x%X / pc -> 0x%X\n", rs, reg[rs], pc);
			jumpFlg = 1;
			funcNum[JR]++;
			break;
		case (ADDU) :	// rd=rs+rt
			rs_original = reg[rs];
			reg[rd] = addu(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\tADDU :\t[$%2u: 0x%2X] + [$%2u: 0x%2X] => [$%2u: 0x%2X]\n", rs, rs_original, rt, reg[rt], rd, reg[rd]);
			funcNum[ADDU]++;
			break;
		case (SUBU) :	// rd=rs-rt
			rs_original = reg[rs];
			reg[rd] = subu(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\tSUBU :\t[$%2u 0x%2X], [$%2u 0x%2X] => [rd:%u] 0x%2X\n", rs, rs_original, rt, reg[rt], rd, reg[rd]);
			funcNum[SUBU]++;
			break;
		case (SLT) :
			if(flag[1] != 1) printf("\tSLT :\t?([$%2u: 0x%04x] < [$%2u: 0x%04x]) ", rs, reg[rs], rt, reg[rt]);
			reg[rd] = slt(reg[rs], reg[rt]);
			if(reg[rd]) {
				if(flag[1] != 1) printf("\t=> [$%2u: 0x%4X] <TRUE>\n", rd, reg[rd]);
			} else {
				if(flag[1] != 1) printf("\t=> [$%2u: 0x%4X] <FALSE>\n", rd, reg[rd]);
			}
			funcNum[SLT]++;
			break;
		case (SLL) :
			if(rs == 0 && shamt == 0) {
				if(flag[1] != 1) printf("\tNOP\n");
				funcNum[NOP]++;
				break;
			}
			reg[rd] = sll(reg[rs], shamt);
			if(flag[1] != 1) printf("\tSLL :\t%u << %u -> %u\n", reg[rs], shamt, reg[rd]);
			funcNum[SLL]++;
			break;
		case (SRL) :
			if(rs == 0 && shamt == 0)
				printf("\tNOP\n");
				funcNum[NOP]++;
				break;
			reg[rd] = srl(reg[rs], shamt);
			if(flag[1] != 1) printf("\tSRL :\t%u << %u -> %u\n", reg[rs], shamt, reg[rd]);
			funcNum[SRL]++;
			break;
		case (AND) :
			reg[rd] = and(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\t[$%2u:0x%4X] AND? [$%2u:0x%4X] \t=> [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			funcNum[AND]++;
			break;
		case (OR) :
			reg[rd] = or(reg[rs], reg[rt]);
			if(flag[1] != 1) printf("\t[$%2u:0x%4X] OR? [$%2u:0x%4X] \t=> [rd:%u] 0x%2X\n", rs, reg[rs], rt, reg[rt], rd, reg[rd]);
			funcNum[OR]++;
			break;
		default :
			printf("Default switch has selected.\n");
	}
	return pc;
}
/* デコーダ */
unsigned int decoder (unsigned int pc, unsigned int instruction, unsigned int* memory,unsigned char* srBuff, unsigned char* serial, unsigned long long breakCount, int* flag, unsigned int* reg, unsigned int* fpreg) {
	unsigned int opcode=0;
	unsigned int rt=0;
	unsigned int rs=0;
	unsigned int rs_original;
	unsigned int im=0;
	unsigned int jump=0;
	unsigned int line=0;
	unsigned int address=0;
	if(instruction != 0 && flag[1] != 1) {
		printf("\t[ops: %06llu,pc: 0x%x]\n", breakCount, pc);
	}
	printf("\t[instruction: 0x%2X]\n", instruction);
	opcode = instruction >> 26;	// opcode: 6bitの整数
	printf("\t[opcode:%2X]\n", opcode);
		rs = (instruction >> 21) & 0x1F;
		rt = (instruction >> 16) & 0x1F;
		im = instruction & 0x0000FFFF;

	/* 適当な時にswitch文に切り替え */
	if(opcode == 0) {
		pc = funct(pc, instruction, flag, reg);
	} else if (opcode == FPU) {
		if(flag[1] != 1) printf("\tFPU \n");
		fpu(pc, instruction, reg, fpreg);
	} else if (opcode == SRCV) {
		if(flag[1] != 1) printf("\tSRCV \n");
		rt = rrb(srBuff);
	} else if (opcode == SSND) {
		if(flag[1] != 1) printf("\tSSND \n");
		rt = (instruction >> 16) & 0x1F;
		rsb(reg[rt], serial);
	} else if (opcode == ADDIU) {
		rs = (instruction >> 21) & 0x1F;
		rt = (instruction >> 16) & 0x1F;
		im = instruction & 0x0000FFFF;
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
		rs = (instruction >> 21) & 0x1F;
		rt = (instruction >> 16) & 0x1F;
		line = instruction & 0xFFFF;	
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
		rs = (instruction >> 21) & 0x1F;
		rt = (instruction >> 16) & 0x1F;
		line = instruction & 0xFFFF;	
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
		rs = (instruction >> 21) & 0x1F;
		rt = (instruction >> 16) & 0x1F;
		im = instruction & 0x0000FFFF;

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
		rs = (instruction >> 21) & 0x1F;
		rt = (instruction >> 16) & 0x1F;
		im = instruction & 0x0000FFFF;

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
	} else if (opcode == JAL) {
		jump = instruction & 0x3FFFFFF;
		jumpFlg = 1;
		if(flag[1] != 1) printf("\tJAL: 0x%04x ($ra <- pc[%X])\n", jump*4, pc+4);
		reg[31] = pc+4;
		pc = jump*4 + PCINIT;
		opNum[JAL]++;
	} else if (opcode == INOUT) {		// シリアルポートから読み込み
		if(flag[1] != 1) printf("\tIN/OUT \n");
//		inout();
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
	unsigned char srBuff[BUFF];	// ファイルから読み込むデータ列(シリアル通信用)
	int srRead=0;
	int fd1 = 0, fd2 = 0;
	unsigned int maxpc;
	unsigned long long breakCount = 0;
	unsigned long long breakpoint = 0x800000;
	unsigned int pc = 0;
	unsigned int *memory;
	unsigned char *serial;
	unsigned int operation;	// 実行中命令

	unsigned int reg[REGSIZE];	// 32 register
	unsigned int fpreg[REGSIZE];	// 32 fpregister
	int i=0;
	unsigned int pAddr = 0;
	unsigned int count = 0;
	unsigned int snum = 0;	// serial portに書き出したbyte数
	int sInFlag=0;
	int flag[32];
	unsigned int mstart = 0, mfinish = 0xFFFFFFFF;
	int orderNum;
	/* flag[0]:help flag[1]:hide flag[5]:sequential  */

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

	for(count=0; count<MEMORYSIZE; count++) {
		memory[count] = 0;
		if(memory[count] != 0) printf("memory\n");
		count++;
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
		if(flag[2] == 0) { sInFlag = i; printf("%s\n", argv[sInFlag+1]); }
		/* printreg */
		flag[3] = strcmp(argv[i], PRINTREG);
		if(flag[3] == 0) { printreg = 1; }
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
	/* ファイルからシリアルポート経由で入力するデータ列(レイトレ元データ)を読み込む */
	if(sInFlag != 0) {	// 比較条件は後で変える
		fd2 = open(argv[sInFlag+1], O_RDONLY);
		if(fd2 < 0) { perror("Unknown error\n"); return 0; }
		srRead = read(fd2, srBuff, BUFF);
		
		if(srRead<0) {
			perror("srBuff failed to read\n");
			return -1;
		} else {
//			printf("opBuff succeeded to read\n");
		}
	}
	count=0;
	if(flag[1] != 1) {
		while(count<40) {
			if(opBuff[count] != 0) printf("%02u(pc:%2X):\t%8X\n", count, (count)*4, opBuff[count]);
			count++;
		}
		while(count<40) {
			if(srBuff[count] != 0) printf("SR %02u:\t%8X\n", count, srBuff[count]);
			count++;
		}
	}

	/* 入力文字列を問答無用でmemoryのPCINIT番地以降にコピーする */
	count = 0;
	printf("maxpc:0x%X(%uline)\n", maxpc+PCINIT, (maxpc)/4);
	while(count < BUFF) {
//		memory[count*4+PCINIT] = opBuff[count];
		if ( opBuff[count] != 0) {
			memory[4*count+3+PCINIT] = opBuff[count] & 0xFF;
			memory[4*count+2+PCINIT] = (opBuff[count] >> 8) & 0xFF;
			memory[4*count+1+PCINIT] = (opBuff[count] >> 16) & 0xFF;
			memory[4*count+PCINIT] = opBuff[count] >> 24;
//			printf("[ DEBUG ]\t%08X, %u\n", opBuff[count], count);
//			printf("[ DEBUG ]\t%02X%02X%02X%02X\n", memory[4*count+PCINIT+3], memory[4*count+PCINIT+2], memory[4*count+PCINIT+1], memory[4*count+PCINIT]);
		}
		count++;
	}

	count=0;

	/* initialize */
	/* register init */
	for(i=0; i<REGSIZE; i++) {
		reg[i] = 0;
	}
	for(i=0;i<128;i++) {
		opNum[i]=0;
	}
	for(i=0;i<128;i++) {
		funcNum[i]=0;
	}
	reg[REGSP] = 0x1FFF4;

	/* Program Counter init */
	pc = PCINIT;
	pAddr = pc;

	
	/* シミュレータ本体 */
	while(pc > PCINIT-1) {	// unsigned int
		if(flag[1] != 1) printf("\n\t====== next: %u ======\n", ((pc-PCINIT)/4));
		reg[0] = 0;
		operation = memory[pAddr] | memory[pAddr+1] << 8 | memory[pAddr+2] << 16 | memory[pAddr+3] << 24;
		pc = decoder(pc, operation, memory, srBuff, serial, breakCount, flag, reg, fpreg);
		if(operation != 0 && printreg != 1) {
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
	while(snum%4 != 0) {
		snum++;
	}

	printf("\nメモリダンプ\n");
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
	printf("Total instructions: %llu\n", breakCount);
	breakCount = breakCount - funcNum[NOP];
	printf("Total instructions (except NOP): %llu\n", breakCount);
	printf("\n(OP)  : \t(Num), \t(Ratio)\n");
	printf("ADDIU : %6u, %03.2f (%%)\n", opNum[ADDIU], (double) 100*opNum[ADDIU]/breakCount);
	printf("LW    : %6u, %03.2f (%%)\n", opNum[LW], (double) 100*opNum[LW]/breakCount);
	printf("SW    : %6u, %03.2f (%%)\n", opNum[SW], (double) 100*opNum[SW]/breakCount);
	printf("JUMP  : %6u, %03.2f (%%)\n", opNum[JUMP], (double) 100*opNum[JUMP]/breakCount);
	printf("JAL   : %6u, %03.2f (%%)\n", opNum[JAL], (double) 100*opNum[JAL]/breakCount);
	printf("BEQ   : %6u, %03.2f (%%)\n", opNum[BEQ], (double) 100*opNum[BEQ]/breakCount);
	printf("BNE   : %6u, %03.2f (%%)\n", opNum[BNE], (double) 100*opNum[BNE]/breakCount);

	printf("JR    : %6u, %03.2f (%%)\n", funcNum[JR], (double) 100*funcNum[JR]/breakCount);
	printf("SUBU  : %6u, %03.2f (%%)\n", funcNum[SUBU], (double) 100*funcNum[SUBU]/breakCount);
	printf("AND   : %6u, %03.2f (%%)\n", funcNum[AND], (double) 100*funcNum[AND]/breakCount);
	printf("OR    : %6u, %03.2f (%%)\n", funcNum[OR], (double) 100*funcNum[OR]/breakCount);
	printf("ADDU  : %6u, %03.2f (%%)\n", funcNum[ADDU], (double) 100*funcNum[ADDU]/breakCount);
	printf("SLT   : %6u, %03.2f (%%)\n", funcNum[SLT], (double) 100*funcNum[SLT]/breakCount);
	printf("SLL   : %6u, %03.2f (%%)\n", funcNum[SLL], (double) 100*funcNum[SLL]/breakCount);
	printf("SRL   : %6u, %03.2f (%%)\n", funcNum[SRL], (double) 100*funcNum[SRL]/breakCount);
	printf("NOP   : %6u\n", funcNum[NOP]);




	snum = 0;
	sInFlag = 0;
	printf("\nシリアルポート出力 ");
	while(snum<BUFF) {
		if (serial[snum] != 0) {
			if(sInFlag == 0) { printf("\n"); }
			sInFlag = 1;
			printf("%c\n", serial[snum]);
		}
		snum++;
	}
//	if(sInFlag == 0) { printf(":無し\n"); }

	free(memory);
	memory = NULL;
	free(serial);
	serial = NULL;

	close(fd1);
	close(fd2);
	return 0;
}

