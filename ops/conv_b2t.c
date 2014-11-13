#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>

#define BUFF 65536u
#define FILESIZE (8lu*1024lu*1024lu)
#define LINE (65536u / 4u)


unsigned int pow2 (unsigned int j) {
	unsigned int retValue = 1;
	while (j > 0) {
		retValue = retValue + retValue;
		j = j - 1;
	}
	return retValue;
}
/* 引数無し */
int main (int argc, char* argv[]) {
	long readCount = 0;
//	unsigned int writeCount = 0;
//	unsigned int countBuff = 0;
	unsigned char *readBuff;
	unsigned int byteCode[BUFF] = { 0 };
	int bitLoc = 0;	// 0<bitLoc<31
	unsigned char textBuff[BUFF];
	unsigned int i, j, temp, btemp;


	/* textBuff初期化 */
	for (j=0; j<(BUFF); j++) {
		textBuff[j] = 0;
	}
	readBuff = (unsigned char *) calloc( FILESIZE, sizeof(char) );
	if(readBuff == NULL) {
		perror("readBuff memory allocation error\n");
		return -1;
	}

/* バイナリコードをファイルから読み込んでreadBuffに書き込む */
/* ファイルには'[^0-4294967295].'と言う形で配置されている */
/* EOFまで読み込む */
		readCount = read(0, readBuff, BUFF);
		if(readCount < 0) perror("read error\n");
		j = 0;
		while(i<readCount && i < BUFF) {
			temp = i % 4;
			switch (temp) {
				case 0: {
					byteCode[j] =  byteCode[j] | (readBuff[i] << 24);	// byteCode:32bit, readBuff:8bit
					break;
				}
				case 1: {
					byteCode[j] =  byteCode[j] | (readBuff[i] << 16 );
					break;
				}
				case 2: {
					byteCode[j] =  byteCode[j] | (readBuff[i] << 8 );
					break;
				}
				case 3: {
					byteCode[j] =  byteCode[j] | (readBuff[i]);
//					printf("%08u ", byteCode[j]);
					j++;
					break;
				}
				default: {
					printf("error\n");
					break;
				}
			}
			i++;
		}

		i=0;
		j=0;
		while(i<readCount) {
			if(byteCode[i] != 0) {
				j=i;
			}
			i++;
		}
//		printf("\nreadCount = %lu\n", readCount);
//		printf("==================================================\n");
//		printf("j=%d, readCount=%ld\n", j, readCount);

		i = 0;
		j++;
		btemp = 0;
		while(i < j*33) {
			bitLoc = 32;
			while(bitLoc >= 0) {
				if(bitLoc == 32) {
					textBuff[(i+bitLoc)] = '\n';
					bitLoc--;
//					printf("bitLoc=%d, \\n\n", bitLoc);
					continue;
				} else if( (byteCode[btemp] % 2) > 0 ) {
					textBuff[(i+bitLoc)] = '1';
//					printf("bitLoc=%d, 1 / ", bitLoc);
				} else {
					textBuff[(i+bitLoc)] = '0';
//					printf("bitLoc=%d, 0 / ", bitLoc);
				}
				byteCode[btemp] = byteCode[btemp] >> 1;
				if(bitLoc == 0) break;
				bitLoc--;
			}
			i = i + 33;
			btemp++;
		}

/* 数字列を標準出力に書き出す */
/* ファイルには'0011001100110011001100110011001111011101110111011101110111011101'と言う形で配置される */
//		printf("textBuff = \n");

		printf("%s\n", textBuff);
		


	return 1;	// 正常終了時１を返す
}


