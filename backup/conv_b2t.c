#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF 65536u
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
	unsigned int readCount = 0, writeCount = 0;
	unsigned int countBuff = 0;
	unsigned char readBuff[BUFF] = { 0 };
	unsigned int byteCode[LINE] = { 0u };
	unsigned int bitLoc = 0;	// 0<bitLoc<31
	unsigned char textBuff[BUFF];
	unsigned int i, j, temp;

	/* readBuff初期化 */	/* byteCode初期化 */
	for (j=0; j<(LINE); j++) {
		byteCode[j] = 0;
	}
	/* textBuff初期化 */
	for (j=0; j<(BUFF); j++) {
		readBuff[j] = 0;
		textBuff[j] = 0;
	}


//	sleep(1);
//	do { /*EOFに到達していない*/

/* バイナリコードをファイルから読み込んでreadBuffに書き込む */
/* ファイルには'[0-4294967295].'と言う形で配置されている */
/* EOFまで読み込む */
		readCount = read(0, readBuff, BUFF);	// readCount [Byte=文字]だけ読み込む(最大65536bytes:unsigend int 16384文字)
		countBuff = readCount;
		printf("[ DEBUG ]\treadCount = %u\n", readCount);
		j = 0;
		while(i<readCount) {
			temp = i % 4;
			switch (temp) {
				case 0: {
					byteCode[j] =  byteCode[j] | readBuff[i];	// byteCode:32bit, readBuff:8bit
					break;
				}
				case 1: {
					byteCode[j] =  byteCode[j] | (readBuff[i] << 8 );
					break;
				}
				case 2: {
					byteCode[j] =  byteCode[j] | (readBuff[i] << 16 );
					break;
				}
				case 3: {
					byteCode[j] =  byteCode[j] | (readBuff[i] << 24);
					j++;
//					printf("%u ", j);
					break;
				}
				default: {
//					printf("default ");
					break;
				}
			}
			i++;
		}


		i = 0;
		temp = 0;
		while(i < readCount/4) {
			j = 0;
			for(bitLoc = 31; bitLoc >= 0; bitLoc--) {
				if( (byteCode[temp] % 2) > 0 ) {
					textBuff[(bitLoc+i)] = '1';
				} else {
					textBuff[(bitLoc+i)] = '0';
				}
//				if((j+1)%32 == 0) printf("(%u, %u), ", i, j);
				byteCode[temp] = byteCode[temp] / 2;
				j++;
				if(j==33) {
					break;
				}
			}
			textBuff[(bitLoc+1+i)] = '\n';	
			i = i + 32;
			temp = i / 32;
		}
//		printf("\n");
/* 数字列を標準出力に書き出す */
/* ファイルには'0011001100110011001100110011001111011101110111011101110111011101'と言う形で配置される */

		printf("%s\n", textBuff);
		
//		printf("[ DEBUG ]\treadCount = %u\n", readCount);
/*
			writeCount = write(1, textBuff, readCount);
			if(writeCount < 1) break;
			readCount = readCount - writeCount;
		printf("\nreadCount = %u\n", readCount);
*/
//		sleep(4);
//	} while (countBuff == BUFF);
//		printf("[ DEBUG ]\tRETURN\n");
	return 1;	// 正常終了時１を返す
}


