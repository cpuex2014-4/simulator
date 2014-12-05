#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "const.h"
#include "alu.h"
#include "print.h"
#include "fpu/C/fpu.h"

unsigned int signExt(unsigned int argument) {
	if(argument > 0x10000) { return argument; }
	if(argument & 0x8000) {
		argument = argument | 0xFFFF8000;
	}
	return argument;
}

unsigned int fpuHide(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, unsigned int* fpuNum, unsigned int* labelRec) {
	unsigned int fpfunction;
	unsigned int fmt, ft, rt, fs, fd, fputemp;

	fmt = (instruction >> 21) & 0x1F;
	ft  = (instruction >> 16) & 0x1F;

	if (fmt == BC1) {
		unsigned int target = instruction & 0xFFFF;
		if (ft == 0) {		// falseで分岐
			fputemp = fpreg[23] & 0x800000;
			if(fputemp == 0) {
				pc = pc + 4 + signExt(target)*4;
				labelRec[pc]++;
			} else { pc = pc + 4; }
			fpuNum[BC1F]++;
		} else if(ft == 1) {	// Trueで分岐
			fputemp = fpreg[23] & 0x800000;
			if(fputemp == 0x800000) {
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
				if(fmt == MFC1M) {
					reg[rt] = fpreg[fs];
					fpuNum[FMFC]++;
				} else if (fmt == MTC1M) {
					fpreg[fs] = reg[rt];
					fpuNum[FMTC]++;
				} else if (fmt == 0x10) {
					fpreg[fd]=fadd (fpreg[fs], fpreg[ft]);
					fpuNum[FADDS]++;
					pc = pc + 4;
					return pc;
				} else {
					fprintf(stderr, "Unknown fmt(function '0').\n");
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
						fputemp = fpreg[fs];
					} else {
						fputemp = fpreg[ft];
					}
					fpreg[fd]=fsub (fpreg[fs], fpreg[ft]);
					fpuNum[FSUBS]++;
				}
				break;
			case (FMULS) :	
				if(fmt == 0x10) { 
					fpreg[fd]=fmul (fpreg[fs], fpreg[ft]);
					fpuNum[FMULS]++;
					pc = pc + 4;
					return pc;
				}
				break;
			case (FDIVS) :	
				if(fmt == 0x10) { 
					if (fd == fs) {
						fputemp = fpreg[fs];
					} else {
						fputemp = fpreg[ft];
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
			default :
				printf("Unknown FPswitch has selected.\n");
		}
		pc = pc + 4;
	}
	return pc;
}



unsigned int fpu(unsigned int pc, unsigned int instruction, unsigned int* reg, unsigned int* fpreg, int* flag, unsigned int* fpuNum, unsigned int* labelRec) {
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

	fpfunction = instruction & 0x3F;
	if (fmt == BC1) {
		target = instruction & 0xFFFF;
		if (ft == 0) {		// falseで分岐
			if(flag[HIDEIND] != 1) { printf("\tMOVS : $FP%02u <- $FP%02u\n", fd, fs); }
			fputemp = fpreg[23] & 0x800000;
			if(fputemp == 0) {
				pc = pc + 4 + signExt(target)*4;
				labelRec[pc]++;
				flag[JUMPFLG] = 1;
				if(flag[HIDEIND] != 1) {
					printf("\tBC1F : (jump_to) -> 0x%X\n", pc);
//					printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
				}
			} else {
				if(flag[HIDEIND] != 1) { printf("\tBC1F : <NOP>\n"); }
			}
			fpuNum[BC1F]++;
		} else if(ft == 1) {	// Trueで分岐
			fputemp = fpreg[23] & 0x800000;
			if(fputemp == 0x800000) {
				pc = pc + 4 + signExt(target)*4;
				labelRec[pc]++;
				printf("\t\tlabelRec(%04X):%u\n", (pc-PCINIT)/4, labelRec[pc]);
				flag[JUMPFLG] = 1;
				if(flag[HIDEIND] != 1) { printf("\tBC1T : (jump_to) -> 0x%X\n", pc); }
			} else {
				if(flag[HIDEIND] != 1) { printf("\tBC1T : <NOP>\n"); }
			}
			fpuNum[BC1T]++;
		} else {
			fprintf(stderr, "\t[ ERROR ]\tUnknown BC1 option(fmt == 0 && (ft != 0 || ft != 1))\n");
		}		
	} else {
		if(instruction != 0 && flag[HIDEIND] != 1) {
			printf("\t[fpfunction:%2X]\n", fpfunction);
		}
		switch (fpfunction) {
			case (0) :
				if(fmt == MFC1M) {
					reg[rt] = fpreg[fs];
					if(flag[HIDEIND] != 1) { printf("\tMFC1 : $%02u <- $FP%02u(%08X)\n", rt, fs, fpreg[fs]); }
					fpuNum[FMFC]++;
				} else if (fmt == MTC1M) {
					fpreg[fs] = reg[rt];
					if(flag[HIDEIND] != 1) { printf("\tMTC1 : $FP%02u <- $%02u(%08X)\n", fs, rt, reg[rt]); }
					fpuNum[FMTC]++;
				} else if (fmt == 0x10) {
					if (fd == fs)
						fputemp = fpreg[fs];
					else
						fputemp = fpreg[ft];
					fpreg[fd]=fadd (fpreg[fs], fpreg[ft]);
					if(flag[HIDEIND] != 1) {
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
					if(flag[HIDEIND] != 1) { printf("\tMOVS : $FP%02u <- $FP%02u\n", fd, fs); }
					fpuNum[MOVSF]++;
				}
				break;
			case (SQRT) :
				/* fd <- sqrt(fs) 	fsqrt  */
				if(fmt == 0x10) { 
					fpreg[fd] = fsqrt(fpreg[fs]);
					if(flag[HIDEIND] != 1) { printf("\tSQRT : ($FP%02u)%X <- SQRT(%02u:%X)\n", fd, fpreg[fd], fs, fpreg[fs]); }
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
					if(flag[HIDEIND] != 1) {
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
					if(flag[HIDEIND] != 1) { printf("\tFMUL : ($FP%02u)%X = ($FP%02u)%X * ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fpreg[fs]); }
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
					if (fd == fs) {
						printf("\tFDIV : ($FP%02u)%X = ($FP%02u)%X / ($FP%02u)%X\n", fd, fpreg[fd], ft, fpreg[ft], fs, fputemp);
					} else {
						printf("\tFDIV : ($FP%02u)%X = ($FP%02u)%X / ($FP%02u)%X\n", fd, fpreg[fd], ft, fputemp, fs, fpreg[fs]);
					}
					fpuNum[FDIVS]++;
				}
				break;
			case (FTOIF) :
				if(fmt == FTOIM) {
					ftoitemp = fpreg[fs];
					fpreg[fd]=ftoi (fpreg[fs]);
					if(flag[HIDEIND] != 1) { printf("\tFTOI :($FP%02u)%X -> ($FP%02u)%X\n", fs, ftoitemp, fd, fpreg[fd]); }
					fpuNum[FTOI]++;
				}
				break;
			case (ITOFF) :
				if(fmt == ITOFM) {
					itoftemp = fpreg[fs];
					fpreg[fd]=itof (fpreg[fs]);
					if(flag[HIDEIND] != 1) { printf("\tITOF : ($FP%02u)%X -> ($FP%02u)%X\n", fs, itoftemp, fd, fpreg[fd]); }
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
					if(flag[HIDEIND] != 1) { printf("\tC.EQ : ?( ($FP%02u)%X == ($FP%02u)%X ) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]); }
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
					if(flag[HIDEIND] != 1) { printf("\tC.OLT : ?( ($FP%02u)%X < ($FP%02u)%X ) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]); }
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
					if(flag[HIDEIND] != 1) { printf("\tC.OLE : ?( ($FP%02u)%X <= ($FP%02u)%X ) -> (cc0)%X\n", fs, fpreg[fs], ft, fpreg[ft], fpreg[23]); }
					fpuNum[COLE]++;
				}
				break;
			default :
				printf("Unknown FPswitch has selected.\n");
		}
	}
	if(flag[HIDEIND] != 1 && flag[PRINTREGIND] == 1) { printFPRegister(fpreg); }
	return pc;
}


unsigned int sll(unsigned int rs, unsigned int shamt) {
	unsigned int rd;
	unsigned int i;

	for(i=0;i<shamt;i++) {
		rs = rs << 1;
	}
	rd = rs;
	return rd;
}
unsigned int srl(unsigned int rs, unsigned int shamt) {
	unsigned int rd;
	unsigned int i;

	for(i=0;i<shamt;i++) {
		rs = rs >> 1;
	}
	rd = rs;
	return rd;
}
unsigned int slt(unsigned int rs, unsigned int rt) {
// slt rs, rt, rd
// R-type
// rs < rt ならばレジスタ rd に 1 を代入、そうでなければ 0 を代入。 
// $rsと$rtの値を符号付き整数として比較し、$rs が小さければ $rd に1を、そうでなければ $rd 0を格納
	unsigned int rd;

	unsigned int urs, urt;
	unsigned int trs, trt;

	if(rs||rt) { ; }

	urs = rs & 0x80000000;
	urt = rt & 0x80000000;
	trs = rs & 0x7FFFFFFF;
	trt = rt & 0x7FFFFFFF;

//	printf("(rs=%X,rt=%X,urs=%X,urt=%X,trs=%X,trt=%X)",rs,rt,urs,urt,trs,trt);

	if(urs == 0 && urt == 0) {
		if(trs < trt) {
			rd = 1;
		} else {
			rd = 0;
		}
	} else if (urs == 0 && urt != 0) {
		rd = 0;
	} else if (urs != 0 && urt == 0) {
		rd = 1;
	} else {
		if(trs < trt) {
			rd = 0;
		} else {
			rd = 1;
		}
	}

	return rd;
}

unsigned int mult(unsigned int rs, unsigned int rt) {
/*	int bit=0;
	int subbit=0;
	unsigned int bitA[32], bitB[32];
	unsigned int bitQ[64] = { 0 };
	unsigned int carry = 0;
*/
	unsigned int rd=0;
	if(rs||rt) { ; }

	return rd;
}


unsigned int or(unsigned int rs, unsigned int rt) {
	// R-type
	// or $rs $rt $rd
	// rd <- rs or rt
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int bitA, bitB;
	unsigned int bitQ = 0;

	/* 各ビットごとにOR演算 */
	while (bit < 32) {
		bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
		bitQ = bitA | bitB;
		rd = rd | (bitQ << bit);
		bit++;
	}
	return rd;
}

unsigned int and(unsigned int rs, unsigned int rt) {
	// R-type
	// and $rs $rt $rd
	// rd <- rs & rt
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int bitA, bitB;
	unsigned int bitQ = 0;

	/* 各ビットごとにAND演算 */
	while (bit < 32) {
		bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
		bitQ = bitA&bitB;
		rd = rd | (bitQ << bit);
		bit++;
	}
	return rd;
}

unsigned int subu (unsigned int rs, unsigned int rt) {
	// R-type
	// subu $rs $rt $rd
	// rd <- rs - rt
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA=0, bitB=0;
	unsigned int bitQ=0;

//	printf("(rt=%ld -> ", (long)rt);
	rt = ~rt;
//	printf("-rt=%ld)", (long)rt);
	
	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		if(bit == 0) {
			c=1;
			bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
			rd = rd | (bitQ << bit);
		} else {
			bitA = ((rs << (31-bit)) >> (31-bit)) >> bit;
			bitB = ((rt << (31-bit)) >> (31-bit)) >> bit;
			bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
			c = (bitA&bitB) | (bitB&c) | (bitA&c);
			rd = rd | (bitQ << bit);
		}
		bit++;
	}
	return rd;
}

unsigned int addiu(unsigned int rs, unsigned int Imm, unsigned int stackPointer) {
	return rs + Imm;
}

unsigned int addu(unsigned int adduA, unsigned int adduB){
	unsigned int rd=0;
	unsigned int bit=0;
	unsigned int c=0;
	unsigned int bitA, bitB;
	unsigned int bitQ=0;

	/* 各ビットごとにOR演算、キャリーがあればフラグ建て */
	while (bit < 32) {
		bitA = ((adduA << (31-bit)) >> (31-bit)) >> bit;
		bitB = ((adduB << (31-bit)) >> (31-bit)) >> bit;

		bitQ = (bitA&bitB&c) | (bitA&~bitB&~c) | (~bitA&bitB&~c) | (~bitA&~bitB&c);
		c = (bitA&bitB) | (bitB&c) | (bitA&c);

		rd = rd | (bitQ << bit);
		bit++;
	}
//	rCarry = c;
	return rd;
}

unsigned int ori(unsigned int rs, unsigned int im) {
	unsigned int rt;

	rt = rs | im;
	return rt;
}



