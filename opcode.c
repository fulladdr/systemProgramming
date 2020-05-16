#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "opcode.h"

//해시 인덱스를 구해서 반환해줌
int makeHashIndex(char *s) {
	int i, total = 0;
	for (i = 0; i < strlen(s); i++) total += s[i];
	return total % 20;
}

//opcode list를 해시 테이블 형식으로 변환해줌
Hashnode** makeHashTable(Hashnode** hashtable) {
	int idx;
	char tnum[5]; char str[11], tstr[11];
	Hashnode* h;
	FILE *fp = fopen("opcode.txt", "r");
	if (fp == NULL) {
		printf("file cannot be opened\n");
		return hashtable;
	}
	while (fscanf(fp, "%s %s %s", tnum, str, tstr) != EOF) {
		Hashnode* tmp = (Hashnode*)calloc(sizeof(Hashnode), 1);
		strcpy(tmp->num, tnum); 
		strcpy(tmp->mnemonic, str);
		strcpy(tmp->type, tstr);
		idx = makeHashIndex(tmp->mnemonic);
		h = hashtable[idx];
		if (!h) hashtable[idx] = tmp;//idx 칸에 아무것도 없으면 해당 값을 바로 할당한다
		else {
			while (h->link)h = h->link;//있을 경우 끝지점까지 link를 타고 이동한다
			h->link = tmp;
		}
	}
	fclose(fp);
	return hashtable;
}
//opcodelist에서 해당 mnemonic을 찾아주는 함수
void printMnemonic(char** tokenInput, Hashnode** hashtable, int TOKEN_SIZE, History* historyHead, History* historyEnd) {
	int idx;
	char cmd[111];
	Hashnode* tmp;
	strcpy(cmd, tokenInput[1]);
	idx = makeHashIndex(cmd);
	tmp = hashtable[idx];
	int flg = 0;
	while (tmp) {
		if (!strcmp(cmd, tmp->mnemonic)) {
			printf("opcode is %02x\n", atoi(tmp->num));
			flg = 1;
			return;
		}
		else tmp = tmp->link;
	}
	if (flg == 0) { //printError(5);
	}//해당 idx 끝까지 봤는데도 없다면 잘못된 mnemonic이다
}
//opcodeList를 출력해주는 함수
void printOpcode(Hashnode** hashtable) {
	int i;
	Hashnode* tmp;
	for (i = 0; i < 20; i++) {
		tmp = hashtable[i];
		printf("%d : ", i);
		while (tmp) {
			printf("[%s,%x]", tmp->mnemonic, atoi(tmp->num));
			if (tmp->link) printf(" -> ");//링크를 타고가 모두 출력해줌
			tmp = tmp->link;
		}
		printf("\n");
	}
}

bool ChkRightFormatOpcode(char* input) {
	int i;
	char inputTmp[INPUT_SIZE];
	strcpy(inputTmp, input);
	i = 0;
	while (inputTmp[i] != ' ') i++;
	i++;
	if (('A' <= inputTmp[i] && inputTmp[i] <= 'Z') || ('a' <= inputTmp[i] && inputTmp[i] <= 'z')) return true;
	//printError(1); 
	return false;
}