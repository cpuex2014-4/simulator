#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF 65536
#define LINE 2048

char textBuff[BUFF];


unsigned int pow2 (unsigned int textLoc) {
	unsigned int retValue = 1;
	while (textLoc > 0) {
		retValue = retValue * 16;
		textLoc = textLoc - 1;
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

/* 引数1:読み込みファイル名 引数2:書き出しファイル名 */
int main (int argc, char* argv[]) {
	int readCount = 0, writeCount = 0;
	int fd[2];
	unsigned int byteCode[LINE] = { 0 };
	unsigned int byteTemp = 0;
	int textCount = 0, line = 0;
	unsigned int textLoc = 0;	// 0<textLoc<8
	unsigned int byteSize;
	unsigned int sizeAll = 0;


	if(argc != 3) { printf("[ ERROR ]\tCheck Arguments.\n"); return -1; }

	if( (fd[0] = open(argv[1], O_RDONLY)) < 0 ) { 
		printf("[ ERROR ]\tFile open error: (%s)\n", argv[1]);
		close(fd[0]);
	}
	if( (fd[1] = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC)) < 0 ) {
		printf("[ ERROR ]\tFile open error: (%s)\n", argv[2]);
		close(fd[1]);
	}


	do { /*EOFに到達していない*/

/* 数字列をファイルから読み込んでtextBuffに書き込む */
/* ファイルには'8fa40000\n27a50004\n...'と言う形で配置されている */
/* EOFまで読み込む */
		if( (readCount = read(fd[0], textBuff, BUFF)) < 0 ) return -1;	// 読み取りバイトが返る
		printf("[ DEBUG ]\treadCount = %u\n", readCount);

//		printf("[ DEBUG ]\n%s\n", textBuff);
		textCount = 0;
		byteTemp = 0;
		textLoc = 8;
		do {
			if(textBuff[textCount] == '\n') {
				byteCode[line] = byteTemp;
				printf("[ %u ] %u\n", line, byteTemp);
				line++;
				textCount++;
//				printf("\\n\n");
				textLoc = 8;
				byteTemp = 0;
				continue;
			}
			if(textBuff[textCount] == '1') {
				byteTemp = byteTemp + pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '2') {
				byteTemp = byteTemp + 0x2*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '3') {
				byteTemp = byteTemp + 0x3*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '4') {
				byteTemp = byteTemp + 0x4*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '5') {
				byteTemp = byteTemp + 0x5*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '6') {
				byteTemp = byteTemp + 0x6*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '7') {
				byteTemp = byteTemp + 0x7*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '8') {
				byteTemp = byteTemp + 0x8*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == '9') {
				byteTemp = byteTemp + 0x9*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == 'a') {
				byteTemp = byteTemp + 0xA*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == 'b') {
				byteTemp = byteTemp + 0xB*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == 'c') {
				byteTemp = byteTemp + 0xC*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == 'd') {
				byteTemp = byteTemp + 0xD*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == 'e') {
				byteTemp = byteTemp + 0xE*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else if(textBuff[textCount] == 'f') {
				byteTemp = byteTemp + 0xF*pow2(textLoc-1);
				printf("[ %c:%u ] %u-> ", textBuff[textCount], line, byteTemp);
			} else { /* printf("\n"); */ }
			
			textLoc--;
			textCount++;
//			sleep(1);
			if(textCount == BUFF) {
				break;
			}
		} while (textBuff[textCount] != 0);

//		printByte(byteCode);
		byteSize = checkSize(byteCode) * sizeof(unsigned int);
//		printf("[ DEBUG ]\tbyteSize = %u\n", byteSize);
/* byteCodeをファイルに書き出す */
/* ファイルには'[0-4294967295]'がEOFまで羅列される */
		do { // 書き終わるまで書き込む
			writeCount = write(fd[1], byteCode, byteSize);
			if(writeCount < 0) break;
			sizeAll = sizeAll + writeCount;
			byteSize = byteSize - writeCount;
			printf("[ DEBUG ]\twriteCount = %u, ", writeCount);
			printf("byteSize = %u\n", byteSize);
		} while (byteSize > 0 );
		line = 0;
	} while (readCount == BUFF);

	close(fd[0]);
	close(fd[1]);
	printf("[ DEBUG ]\tsizeAll = %u\n", sizeAll);
	return 1;	// 正常終了時１を返す
}


