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
	unsigned int readCount = 0;
//	unsigned int writeCount = 0;
//	unsigned int countBuff = 0;
	unsigned char readBuff[BUFF] = { 0 };
	unsigned int byteCode[LINE] = { 0u };
	unsigned int bitLoc = 0;	// 0<bitLoc<31
	unsigned char textBuff[BUFF];
	unsigned int i, j, temp, btemp;

	/* readBuff初期化 */	/* byteCode初期化 */
	for (j=0; j<(LINE); j++) {
		byteCode[j] = 0;
	}
	/* textBuff初期化 */
	for (j=0; j<(BUFF); j++) {
		readBuff[j] = 0;
		textBuff[j] = 0;
	}


/* バイナリコードをファイルから読み込んでreadBuffに書き込む */
/* ファイルには'[0-4294967295].'と言う形で配置されている */
/* EOFまで読み込む */
		readCount = read(0, readBuff, BUFF);	// readCount [Byte=文字]だけ読み込む(最大65536bytes:unsigend int 16384文字)
//		countBuff = readCount;
		j = 0;
		while(i<readCount*4) {
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
					break;
				}
				default: {
					break;
				}
			}
			i++;
		}


		i = 0;
		btemp = 0;
		while(i < readCount*33/4) {
			bitLoc = 32;
			while(bitLoc == 0) {
				if(bitLoc == 32) {
					textBuff[(bitLoc+i)] = '\n';
				} else if( (byteCode[btemp] % 2) > 0 ) {
					textBuff[(bitLoc+i)] = '1';
				} else {
					textBuff[(bitLoc+i)] = '0';
				}
				byteCode[btemp] = byteCode[btemp] / 2;

//				printf("%u\n", bitLoc);
				bitLoc--;
			}
			i = i + 33;
			btemp++;
		}

/* 数字列を標準出力に書き出す */
/* ファイルには'0011001100110011001100110011001111011101110111011101110111011101'と言う形で配置される */

		printf("%s\n", textBuff);
		


	return 1;	// 正常終了時１を返す
}


