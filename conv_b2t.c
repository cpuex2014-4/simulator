#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF 65536
#define LINE 2048


/*
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
*/
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
	int readCount = 0, writeCount = 0;
	unsigned char readBuff[LINE] = { 0 };
	unsigned int byteCode[LINE] = { 0u };
	unsigned int byteTemp = 0;
	int bitCount = 0, line = 0;
	unsigned int bitLoc = 0;	// 0<bitLoc<31
	unsigned char textBuff[BUFF];
	unsigned int byteSize;
	unsigned int i, j, temp;

	/* readBuff初期化 */	/* byteCode初期化 */
	for (j=0; j<(LINE); j++) {
		readBuff[j] = 0;
		byteCode[j] = 0;
	}
	/* textBuff初期化 */
	for (j=0; j<(BUFF); j++) {
		textBuff[j] = 0;
	}


//	sleep(1);
	do { /*EOFに到達していない*/

/* バイナリコードをファイルから読み込んでreadBuffに書き込む */
/* ファイルには'[0-4294967295].'と言う形で配置されている */
/* EOFまで読み込む */
		readCount = read(0, readBuff, LINE);
//		printf("[ DEBUG ]\treadCount = %u\n", readCount);
		j = 0;
		while(i<LINE) {
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
//					printf("default ");
					break;
				}
			}
			i++;
		}
//		printf("\n");
//		printf("[ DEBUG ]\tbyteCode = \n");
//		for (j=0; j<(LINE/4); j++) {
//			if(byteCode[j] != 0) printf("[%u/", byteCode[j]);
//			if(byteCode[j] != 0) printf("%x], ", byteCode[j]);
//		}
//		printf("\n");
//		sleep(10);
		bitCount = 0;
		byteTemp = 0;
		i = 0;
		temp = 0;
		while(i < LINE) {
//			if(temp<6) printf("\nbyteCode[%u] = %u", temp, byteCode[temp]);
			j = 0;
			for(bitLoc = 31; bitLoc >= 0; bitLoc--) {
//				if(temp < 6) printf("\n[ DEBUG ]\ti=%d, bitLoc=%d, byteCode[temp]=%u, pow2(%d)=%u, ", i, bitLoc, byteCode[temp], j, pow2(j));
				if( (byteCode[temp] % 2) > 0 ) {
					textBuff[(bitLoc+i)] = '1';
//					if(temp < 6) printf("textBuff[%d]=%c, temp=%d", bitLoc+i, textBuff[bitLoc+i], temp);
				} else {
					textBuff[(bitLoc+i)] = '0';
//					if (temp<6) printf("textBuff[%d]=%c, temp=%d", bitLoc+i, textBuff[bitLoc+i], temp);
				}
				byteCode[temp] = byteCode[temp] / 2;
				j++;
				if(j==32) break;
			}
			i = i + 32;
			temp = i / 32;
		}
//		printf("\n[ DEBUG ]\t%s\n", textBuff);
//		printf("\n");

//		printByte(byteCode);
//		byteSize = checkSize(byteCode) * sizeof(unsigned int);
//		printf("[ DEBUG ]\tbyteSize = %u\n", byteSize);
/* 数字列を標準出力に書き出す */
/* ファイルには'0011001100110011001100110011001111011101110111011101110111011101'と言う形で配置される */

//		byteSize = sizeof(textBuff);

		for(i=0;i<BUFF;i++) {
			if(textBuff[i]!=0) byteSize = i;
		}

//		do { // 書き終わるまで書き込む
			writeCount = write(1, textBuff, byteSize);
			if(writeCount < 0) break;
			byteSize = byteSize - writeCount;
//			printf("[ DEBUG ]\twriteCount = %u, ", writeCount);
//			printf("byteSize = %u\n", byteSize);
//		} while ( byteSize > 0 );
		line = 0;
	} while (readCount == BUFF);

	return 1;	// 正常終了時１を返す
}


