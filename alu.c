#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "const.h"
#include "alu.h"
#include "print.h"
#include "fpu/C/fpu.h"

unsigned int signExt(unsigned int argument) {
/*	if(argument & 0x8000) {
		argument = argument | 0xFFFF8000;
	}
*/
//	int diff = (temp >= (1<<15)) ? temp-(1<<16) : temp; //diff はtempの符号拡張
	argument = (argument >= (1<<15)) ? argument - (1<<16) : argument;
	return argument;
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

	memory[address+3] = addr3;
	memory[address+2] = addr2;
	memory[address+1] = addr1;
	memory[address] = addr0;

/*	if(rt != ( memory[address] | memory[address+1] << 8 | memory[address+2] << 16 | memory[address+3] << 24 )) {
		fprintf(stderr, "\n[ ERROR ]\tFailed to SW\n");
		exit(1);
	}
*/	return 0;
}
/* load word */
// lw rs,rt,Imm => R(rt) <- M[ R(rs)+Imm ]	メモリのR(rs)+Imm番地の内容をrtに書き込む
unsigned int lw(unsigned int address, unsigned int* memory) {
	unsigned rt = 0;
	rt = memory[address] | memory[address+1] << 8 | memory[address+2] << 16 | memory[address+3] << 24;
	return rt;
}


unsigned int fpuHide(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, unsigned long long* fpuNum, unsigned int* labelRec) {
	unsigned int fpfunction;
	unsigned int fmt, ft, rt, fs, fd, fpregtemp;
	unsigned int target;

	fmt = (instruction >> 21) & 0x1F;
	ft  = (instruction >> 16) & 0x1F;

	if (fmt == BC1) {
		target = instruction & 0xFFFF;
		fpregtemp = fpreg[23] & 0x800000;
		if (ft == 0) {		// falseで分岐
			if(fpregtemp == 0) {
				pc = pc + 4 + signExt(target)*4;
				labelRec[pc]++;
			} else { pc = pc + 4; }
			fpuNum[BC1F]++;
		} else if(ft == 1) {	// Trueで分岐
			if(fpregtemp == 0x800000) {
				pc = pc + 4 + signExt(target)*4;
				labelRec[pc]++;
			} else { pc = pc + 4; }
			fpuNum[BC1T]++;
		} else {
			fprintf(stderr, "\t[ ERROR ]\tUnknown BC1 option(fmt == 0 && (ft != 0 || ft != 1))\n");
			pc = pc + 4;
		}		
	} else {
		fpfunction = instruction & 0x3F;
		fs  = (instruction >> 11 ) & 0x1F;
		fd  = (instruction >> 6 ) & 0x1F;
		switch (fpfunction) {
			case (0) :
				rt  = (instruction >> 16) & 0x1F;
				switch (fmt) {
					case (MFC1M) :
						reg[rt] = fpreg[fs];
						fpuNum[FMFC]++;
						break;
					case (MTC1M) :
						fpreg[fs] = reg[rt];
						fpuNum[FMTC]++;
						break;
					case (0x10) :
						// fadd
						fpreg[fd]=fadd (fpreg[fs], fpreg[ft]);
						fpuNum[FADDS]++;
						break;
					default :
						fprintf(stderr, "Unknown fmt(function '0').\n");
						break;
				}
/*				if(fmt == MFC1M) {
					reg[rt] = fpreg[fs];
					fpuNum[FMFC]++;
				} else if (fmt == MTC1M) {
					fpreg[fs] = reg[rt];
					fpuNum[FMTC]++;
				} else if (fmt == 0x10) {
					// fadd
					fpreg[fd]=fadd (fpreg[fs], fpreg[ft]);
					fpuNum[FADDS]++;
				} else {
					fprintf(stderr, "Unknown fmt(function '0').\n");
				}
*/				break;
			case (FMULS) :	
				if(fmt == 0x10) { 
					fpreg[fd]=fmul (fpreg[fs], fpreg[ft]);
					fpuNum[FMULS]++;
				}
				break;
			case (COLE) :
				if(fmt == 0x10) {
					if(fle (fpreg[fs], fpreg[ft])) {
						fpreg[23] = fpreg[23] | 0x800000;
					} else {
						fpreg[23] = fpreg[23] & 0xFF7FFFFF;
					}
					fpuNum[COLE]++;
				}
				break;
			case (MOVSF) :	
				if(fmt == 0x10) { 
					fpreg[fd] = fpreg[fs];
					fpuNum[MOVSF]++;
				}
				break;
			case (SQRT) :
				rt  = (instruction >> 16) & 0x1F;
				if(fmt == 0x10) { 
					fpreg[fd] = fsqrt(fpreg[fs]);
					fpuNum[SQRT]++;
				}
				break;
			case (FSUBS) :
				if(fmt == 0x10) { 
					if (fd == fs) {
						fpregtemp = fpreg[fs];
					} else {
						fpregtemp = fpreg[ft];
					}
					fpreg[fd]=fsub (fpreg[fs], fpreg[ft]);
					fpuNum[FSUBS]++;
				}
				break;
			case (FDIVS) :	
				if(fmt == 0x10) { 
					if (fd == fs) {
						fpregtemp = fpreg[fs];
					} else {
						fpregtemp = fpreg[ft];
					}
					fpreg[fd]=fdiv (fpreg[fs], fpreg[ft]);
					fpuNum[FDIVS]++;
				}
				break;
			case (FTOIF) :
				if(fmt == FTOIM) {
					fpreg[fd]=ftoi (fpreg[fs]);
					fpuNum[FTOI]++;
				}
				break;
			case (ITOFF) :
				if(fmt == ITOFM) {
					fpreg[fd]=itof (fpreg[fs]);
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
					fpuNum[COLT]++;
				}
				break;
			default :
				fprintf(stderr, "Unknown FPswitch has selected.\n");
		}
		pc = pc + 4;
	}
	return pc;
}


/*
unsigned int fpu(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, int* flag, unsigned long long* fpuNum, unsigned int* labelRec) {
	unsigned int fpfunction, fmt, ft, rt, fs, fd, itoftemp, ftoitemp, fpregtemp, target;
	fmt = (instruction >> 21) & 0x1F;
	ft  = (instruction >> 16) & 0x1F;
	rt  = (instruction >> 16) & 0x1F;
	fs  = (instruction >> 11 ) & 0x1F;
	fd  = (instruction >> 6 ) & 0x1F;

	fpfunction = instruction & 0x3F;
	if (fmt == BC1) {
		target = instruction & 0xFFFF;
		if (ft == 0) {		// falseで分岐
			if(flag[HIDEIND] == 1) { printf("\tMOVS : $FP%02u <- $FP%02u\n", fd, fs); }
			if((fpreg[23] & 0x800000) == 0) {
				pc = pc + 4 + signExt(target)*4;
				labelRec[pc]++;
				flag[JUMPFLG] = 1;
				if(flag[HIDEIND] == 1) {
					printf("\tBC1F : (jump_to) -> 0x%X\n", pc);
//					printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
				}
			} else {
				if(flag[HIDEIND] == 1) { printf("\tBC1F : <NOP>\n"); }
			}
			fpuNum[BC1F]++;
		} else if(ft == 1) {	// Trueで分岐
			if((fpreg[23] & 0x800000) == 0x800000) {
				pc = pc + 4 + signExt(target)*4;
				labelRec[pc]++;
				printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
				flag[JUMPFLG] = 1;
				if(flag[HIDEIND] == 1) { printf("\tBC1T : (jump_to) -> 0x%X\n", pc); }
			} else {
				if(flag[HIDEIND] == 1) { printf("\tBC1T : <NOP>\n"); }
			}
			fpuNum[BC1T]++;
		} else {
			fprintf(stderr, "\t[ ERROR ]\tUnknown BC1 option(fmt == 0 && (ft != 0 || ft != 1))\n");
		}		
	} else {
		if(instruction != 0 && flag[HIDEIND] == 1) {
			printf("\t[fpfunction:%2X]\n", fpfunction);
		}
		switch (fpfunction) {
			case (0) :
				if(fmt == MFC1M) {
					reg[rt] = fpreg[fs];
					if(flag[HIDEIND] == 1) { printf("\tMFC1 : $%02u <- $FP%02u(%08X)\n", rt, fs, fpreg[fs]); }
					fpuNum[FMFC]++;
				} else if (fmt == MTC1M) {
					fpreg[fs] = reg[rt];
					if(flag[HIDEIND] == 1) { printf("\tMTC1 : $FP%02u <- $%02u(%08X)\n", fs, rt, reg[rt]); }
					fpuNum[FMTC]++;
				} else if (fmt == 0x10) {
					if (fd == fs) {
						fpregtemp = fpreg[fs];
					} else {
						fpregtemp = fpreg[ft];
					}
					fpreg[fd]=fadd (fpreg[fs], fpreg[ft]);
					if(flag[HIDEIND] == 1) {
						if (fd == fs) {
							printf("\tFADD : ($FP%02u)%X = ($FP%02u)%X + ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fpregtemp);
						} else {
							printf("\tFADD : ($FP%02u)%X = ($FP%02u)%X + ($FP%02u)%X\n", fd, fpreg[fd], ft, fpregtemp, fs, fpreg[fs]);
						}
					}
					fpuNum[FADDS]++;
				} else {
					printf("Unknown fmt(function '0').\n");
				}
				break;
			case (FMULS) :	
				if(fmt == 0x10) { 
					fpreg[fd]=fmul (fpreg[fs], fpreg[ft]);
					if(flag[HIDEIND] == 1) { printf("\tFMUL : ($FP%02u)%X = ($FP%02u)%X * ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fpreg[fs]); }
					fpuNum[FMULS]++;
				}
				break;
			case (COLE) :
				if(fmt == 0x10) {
					if(fle (fpreg[fs], fpreg[ft])) {
						fpreg[23] = fpreg[23] | 0x800000;
					} else {
						fpreg[23] = fpreg[23] & 0xFF7FFFFF;
					}
					if(flag[HIDEIND] == 1) { printf("\tC.OLE : ?( ($FP%02u)%X <= ($FP%02u)%X ) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]); }
					fpuNum[COLE]++;
				}
				break;
			case (MOVSF) :	
				if(fmt == 0x10) { 
					fpreg[fd] = fpreg[fs];
					if(flag[HIDEIND] == 1) { printf("\tMOVS : $FP%02u <- $FP%02u\n", fd, fs); }
					fpuNum[MOVSF]++;
				}
				break;
			case (SQRT) :
				// fd <- sqrt(fs) 	fsqrt
				if(fmt == 0x10) { 
					fpreg[fd] = fsqrt(fpreg[fs]);
					if(flag[HIDEIND] == 1) { printf("\tSQRT : ($FP%02u)%X <- SQRT(%02u:%X)\n", fd, fpreg[fd], fs, fpreg[fs]); }
					fpuNum[SQRT]++;
				}
				break;
			case (FSUBS) :
				if(fmt == 0x10) { 
					if (fd == fs) {
						fpregtemp = fpreg[fs];
					} else {
						fpregtemp = fpreg[ft];
					}
					fpreg[fd]=fsub (fpreg[fs], fpreg[ft]);
					if(flag[HIDEIND] == 1) {
						if (fd == fs)
							printf("\tFSUB : ($FP%02u)%X = ($FP%02u)%X - ($FP%02u)%X\n", fd, fpreg[fd], fs, fpregtemp, ft, fpreg[ft]);
						else
							printf("\tFSUB : ($FP%02u)%X = ($FP%02u)%X - ($FP%02u)%X\n", fd, fpreg[fd], fs, fpreg[fs], ft, fpregtemp);
					}
					fpuNum[FSUBS]++;
				}
				break;
			case (FDIVS) :	
				if(fmt == 0x10) { 
					if (fd == fs) {
						fpregtemp = fpreg[fs];
					} else {
						fpregtemp = fpreg[ft];
					}
					fpreg[fd]=fdiv (fpreg[fs], fpreg[ft]);
					if (fd == fs) {
						printf("\tFDIV : ($FP%02u)%X = ($FP%02u)%X / ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fpregtemp);
					} else {
						printf("\tFDIV : ($FP%02u)%X = ($FP%02u)%X / ($FP%02u)%X\n", fd, fpreg[fd], ft, fpregtemp, fs, fpreg[fs]);
					}
					fpuNum[FDIVS]++;
				}
				break;
			case (FTOIF) :
				if(fmt == FTOIM) {
					ftoitemp = fpreg[fs];
					fpreg[fd]=ftoi (fpreg[fs]);
					if(flag[HIDEIND] == 1) { printf("\tFTOI :($FP%02u)%X -> ($FP%02u)%X\n", fs, ftoitemp, fd, fpreg[fd]); }
					fpuNum[FTOI]++;
				}
				break;
			case (ITOFF) :
				if(fmt == ITOFM) {
					itoftemp = fpreg[fs];
					fpreg[fd]=itof (fpreg[fs]);
					if(flag[HIDEIND] == 1) { printf("\tITOF : ($FP%02u)%X -> ($FP%02u)%X\n", fs, itoftemp, fd, fpreg[fd]); }
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
					if(flag[HIDEIND] == 1) { printf("\tC.EQ : ?( ($FP%02u)%X == ($FP%02u)%X ) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]); }
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
					if(flag[HIDEIND] == 1) { printf("\tC.OLT : ?( ($FP%02u)%X < ($FP%02u)%X ) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]); }
					fpuNum[COLT]++;
				}
				break;
			default :
				fprintf(stderr, "Unknown FPswitch has selected.\n");
		}
	}
	if(flag[HIDEIND] == 1 && flag[PRINTREGIND] == 1) { printFPRegister(fpreg); }
	return pc;
}
*/

unsigned int sll(unsigned int rs, unsigned int shamt) {
	unsigned int i;

	for(i=0;i<shamt;i++) {
		rs = rs << 1;
	}
	return rs;
}
/*
Format: SLLV rd, rt, rs MIPS32
Purpose: To left-shift a word by a variable number of bits
Description: rd <- rt << rs
	The contents of the low-order 32-bit word of GPR rt are shifted left, inserting zeros into the emptied bits; the result
	word is placed in GPR rd. The bit-shift amount is specified by the low-order 5 bits of GPR rs.
Restrictions: None
Operation:
	s <- GPR[rs]4..0
	temp <- GPR[rt](31-s)..0 || 0s
	GPR[rd] <- temp
*/
unsigned int sllv(unsigned int rs, unsigned int rt) {
	unsigned int s,rd;

	s = rs & 0xF;
	rd = (rt << ( 31 - s ));
	
	return rd;
}

unsigned int srl(unsigned int rs, unsigned int shamt) {
	unsigned int i;

	for(i=0;i<shamt;i++) {
		rs = rs >> 1;
	}
	return rs;
}

unsigned int sra(unsigned int rs, unsigned int shamt) {
	unsigned int rd;

	if(rs & 0x8000) {
		rd = ((rs & 0x7FFFFFFF) >> shamt) | 0x80000000;
	} else {
		rd = rs >> shamt;
	}

	return rd;
}

unsigned int slt(unsigned int rs, unsigned int rt) {
// slt rs, rt, rd
// R-type
// rs < rt ならばレジスタ rd に 1 を代入、そうでなければ 0 を代入。 
// $rsと$rtの値を符号付き整数として比較し、$rs が小さければ $rd に1を、そうでなければ $rd 0を格納
	
//	unsigned int trs, trt;
//	unsigned int rd;
	int srs, srt;

	srs = (int) rs;
	srt = (int) rt;
	if(srs < srt) {
		return 1;
	} else {
		return 0;
	}

/*	unsigned int urs, urt;

	urs = rs & 0x80000000;
	urt = rt & 0x80000000;
	trs = rs & 0x7FFFFFFF;
	trt = rt & 0x7FFFFFFF;

	if(urs == 0 && urt == 0) {
		if(trs < trt) {
			return 1;
		} else {
			return 0;
		}
	} else if (urs == 0 && urt != 0) {
		return 0;
	} else if (urs != 0 && urt == 0) {
		return 1;
	} else {
		if(trs < trt) {
			return 0;
		} else {
			return 1;
		}
	}
*/
}

unsigned int slti(unsigned int rs, unsigned int im) {
	int signedIm;
	int signedRs;

	signedIm = (int) signExt(im);
	signedRs = (int) rs;
	if(signedRs < signedIm) {
		return 1;
	} else {
		return 0;
	}
	
}
unsigned int sltiu(unsigned int rs, unsigned int im) {
	if(rs < signExt(im)) {
		return 1;
	} else {
		return 0;
	}
	
}

unsigned int or(unsigned int rs, unsigned int rt) {
	// R-type
	// or $rs $rt $rd
	// rd <- rs or rt
	unsigned int rd;

	rd = rs | rt;
	return rd;
}

unsigned int and(unsigned int rs, unsigned int rt) {
	// R-type
	// and $rs $rt $rd
	// rd <- rs & rt
	unsigned int rd;
	rd = rs & rt;
	return rd;
}

unsigned int andi(unsigned int rs, unsigned int im) {
	// imはzero_ext
	unsigned int rd;
	rd = rs & im;
	return rd;
}

unsigned int subu (unsigned int rs, unsigned int rt) {
	// R-type
	// subu $rs $rt $rd
	// rd <- rs - rt
	unsigned int rd;
	rd = rs - rt;
	return rd;
}

unsigned int addiu(unsigned int rs, unsigned int im) {
	return rs + im;
}

unsigned int addu(unsigned int rs, unsigned int rt){
	return (rs + rt);
}

unsigned int ori(unsigned int rs, unsigned int im) {
	unsigned int rt;

	rt = rs | im;
	return rt;
}

unsigned int nor(unsigned int rs, unsigned int rt) {
	unsigned int rd;

	rd = !rs & !rt;
	return rd;
}

unsigned int xor(unsigned int rs, unsigned int rt) {
	return (rs ^ rt);
}
unsigned int xori(unsigned int rs, unsigned int im) {
	//imはzero_ext

	return (rs ^ im);
}



