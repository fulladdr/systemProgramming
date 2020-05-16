#include "20170795.h"
#include "memory.h"

//메모리 표시해주는 dump 함수
bool functionDump(int TOKEN_SIZE, char** tokenInput, int* memory, int* lastAddr) {
	int start, end, i, j;
	if (TOKEN_SIZE == 1) {
		start = *lastAddr;
		end = MIN(0xFFFFF, start + 159);
	}
	else if (TOKEN_SIZE == 2) {
		if (!isHexa(tokenInput[1])) {
			printError(4);
			return false;
		}
		sscanf(tokenInput[1], "%x", &start);
		if (start > 0xFFFFF) { printError(4); return false; }
		end = MIN(0xFFFFF, start + 159);
	}
	else if (TOKEN_SIZE == 3) {
		if (!isHexa(tokenInput[1]) || !isHexa(tokenInput[2])) {
			printError(4);
			return false;
		}
		sscanf(tokenInput[1], "%x", &start);
		sscanf(tokenInput[2], "%x", &end);
		if (start > 0xFFFFF || end > 0xFFFFF || end < start) {
			printError(4);
			return false;
		}
	}
	for (i = start / 16; i <= end / 16; i++) {
		printf("%05X\t", i * 16);
		for (j = 0; j < 16; j++) {
			int tmp = i * 16 + j;//메모리 주소를 각각 가리킴
			if (tmp >= start && tmp <= end) {
				printf("%02X  ", memory[tmp]);//메모리 주소에 적힌 값 출력
			}
			else printf("    ");
		}
		printf("; ");
		for (j = 0; j < 16; j++) {
			if (i * 16 + j >= start && i * 16 + j <= end) {
				if (0x20 <= memory[i * 16 + j] && memory[i * 16 + j] <= 0x7E) {//아스키코드 범위안에 있다면
					printf("%c", (char)memory[i * 16 + j]);
				}
				else printf(".");
			}
			else printf(".");
		}
		printf("\n");
	}
	*lastAddr = end + 1;
	if (*lastAddr > 0xFFFFF) *lastAddr = 0;
	return true;
}

//메모리 값을 edit해주는 함수
bool functionEdit(char** tokenInput, int* memory) {
	int data, target;
	if (tokenInput[1] == NULL || tokenInput[2] == NULL) { printError(2); return false; }
	if (!isHexa(tokenInput[1]) || !isHexa(tokenInput[2])) { printError(2); return false; }
	sscanf(tokenInput[1], "%x", &target);
	sscanf(tokenInput[2], "%x", &data);
	if (target > 0xfffff) { printError(4); return false; }
	if (data < 0 || data>255) { printError(8); return false; }
	memory[target] = data;//메모리 타겟 자리에 데이터를 써줌
	return true;
}

//메모리를 어떤 값으로 채워주는 함수
bool functionFill(char** tokenInput, int* memory) {
	int start, end, value;
	if (tokenInput[1] == NULL || tokenInput[2] == NULL || tokenInput[3] == NULL) { printError(2); return false; }
	if (!isHexa(tokenInput[1]) || !isHexa(tokenInput[2]) || !isHexa(tokenInput[3])) { printError(2); return false; }
	sscanf(tokenInput[1], "%x", &start);
	sscanf(tokenInput[2], "%x", &end);
	sscanf(tokenInput[3], "%x", &value);
	if (start < 0 || start>0xfffff || end < 0 || end>0xfffff || start > end) { printError(4); return false; }
	if (value < 0 || value>255) { printError(8); return false; }
	for (int i = start; i <= end; i++) {
		memory[i] = value;
	}
	return true;
}
//메모리를 리셋해줌
void resetMemory(int* memory) {
	for (int i = 0; i < MEMORY_SIZE; i++)memory[i] = 0;
}