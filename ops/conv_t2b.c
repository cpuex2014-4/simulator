#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF 65536
#define LINE 2048

char textBuff[BUFF];


unsigned int pow2 (unsigned int endian) {
	unsigned int retValue = 1;
	while (endian > 0) {
		retValue = retValue + retValue;
		endian = endian - 1;
	}
	return retValue;
}

void printByte(unsigned int* byteCode) {
	int line;
	int size;

	printf("[ DEBUG ]\tprintByte : \n");
	size = LINE;
	for(line = 0; line < size; line++ ) {
		if(byteCode[line] != 0) {
			printf("[%d]=%u ", line, byteCode[line]);
		}
	}
	printf("\n");
}

unsigned int checkSize(unsigned int* byteCode) {
	unsigned int size;
	unsigned int i;

	for(i = 0; i < LINE; i++) {
		if(byteCode[i] != 0u) {
			size = i+1;
		}
	}
	return size;
}

/* 標準入力 -> 標準出力 */
int main (int argc, char* argv[]) {
	int readCount = 0, writeCount = 0;
	unsigned int byteCode[LINE] = { 0 };
	unsigned int byteTemp = 0;
	int textCount = 0, line = 0;
	int bitLocation = 0;	// 0<bitLocation<31
	unsigned int byteSize;
	unsigned int sizeAll = 0;
	int endian = 7;
	int comment = 0;	// '#'で1、'\n'で0

/*	if(argc != 3) { printf("[ ERROR ]\tCheck Arguments.\n"); return -1; }

	if( (fd[0] = open(argv[1], O_RDONLY)) < 0 ) { 
		printf("[ ERROR ]\tFile open error: (%s)\n", argv[1]);
		close(fd[0]);
	}
	if( (fd[1] = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0777)) < 0 ) {
		printf("[ ERROR ]\tFile open error: (%s)\n", argv[2]);
		close(fd[1]);
	}
*/

	do { /*EOFに到達していない*/

/* 01<ASCII>列をファイルから読み込んでunsigned intに変換し、textBuffに書き込む */
/* EOFまで読み込む */
		if( (readCount = read(0, textBuff, BUFF)) < 0 ) return -1;	// 読み取りバイトが返る
//		printf("[ DEBUG ]\treadCount = %u\n", readCount+1);

//		printf("[ DEBUG ]\n%s\n", textBuff);
		textCount = 0;
		byteTemp = 0;
		bitLocation = 0;
		do {
//			if(bitLocation == 0) printf("line = %d\n", line);
			if((bitLocation > 31 && textBuff[textCount] != '\n') || textBuff[textCount] == ' ' || comment == 1) {
				textCount++;
				continue;
			} else if (textBuff[textCount] == '\n') {
//				printf(" : byteTemp = 0x%X\n",byteTemp);
//				printf("\n\n");
				byteCode[line] = byteTemp;
				line++;
				textCount++;
				bitLocation = 0;
				byteTemp = 0;
				comment = 0;
				endian = 7;
				continue;
			} else if (textBuff[textCount] == '#') {
				comment = 1;
				textCount++;
				continue;
			}
			
			if(textBuff[textCount] == '1') {
				switch (bitLocation / 8) {
					case(0) :
						byteTemp = byteTemp + pow2(endian);
						break;
					case(1) :
						byteTemp = byteTemp + pow2(endian+8);
						break;
					case(2) :
						byteTemp = byteTemp + pow2(endian+16);
						break;
					case(3) :
						byteTemp = byteTemp + pow2(endian+24);
						break;
					default:
						break;
				}
//				printf("1:%d(0x%8X) ", endian, byteTemp);
			} else { 
//				printf("0:%d(0x%8X) ", endian, byteTemp);
			}
			textCount++;
			bitLocation++;
			if(endian <= 0) {
				endian = 8;
//				printf("\n");
			}
			endian--;
		} while (textBuff[textCount] != 0);


//		printByte(byteCode);
		byteSize = checkSize(byteCode) * sizeof(unsigned int);
//		printf("[ DEBUG ]\tbyteSize = %u\n", byteSize);
/* byteCodeをファイルに書き出す */
/* ファイルには'[0-4294967295]'がEOFまで羅列される */
		do { // 書き終わるまで書き込む
			writeCount = write(1, byteCode, byteSize);
			if(writeCount < 0) break;
			sizeAll = sizeAll + writeCount;
			byteSize = byteSize - writeCount;
//			printf("[ DEBUG ]\twriteCount = %u, ", writeCount);
//			printf("byteSize = %u\n", byteSize);
		} while (byteSize > 0 );
		line = 0;
	} while (readCount == BUFF);
/*
	close(fd[0]);
	close(fd[1]);
*/
//	printf("[ DEBUG ]\tsizeAll = %u\n", sizeAll);
	return 1;	// 正常終了時１を返す
}


