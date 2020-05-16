#include "20170795.h"
#include "memory.h"
#include "opcode.h"
#include "shell.h"
#include "assembler.h"

bool assembleFlag = false;
History *historyHead = NULL;
History *historyEnd = NULL;

//input을 tokenize한다
bool tokenize(int *TOKEN_SIZE, char* input, char** tokenInput) {
	char inputLine[INPUT_SIZE];
	char* ptr;
	int i;
	strcpy(inputLine, input);
	ptr = strtok(inputLine, " ,\n\t\r");
	if (!ptr) return false;
	tokenInput[0] = (char*)calloc(sizeof(char)*(INPUT_SIZE + 1), 1);
	strcpy(tokenInput[0], ptr);
	//tokenInput에 tokenize한 값을 index 0, 1, 2 ... 순서로 메모리 할당 후 저장한다
	i = 1;
	while (1) {
		ptr = strtok(NULL, " ,\n\t\r");
		if (ptr == NULL)break;
		tokenInput[i] = (char*)calloc(sizeof(char)*(INPUT_SIZE + 1), 1);
		strcpy(tokenInput[i], ptr);
		i++;
	}
	*TOKEN_SIZE = i;
	return true;
}

//다음 명령에 넘어가기 전에 전역변수를 초기화한다
void setMemory(int TOKEN_SIZE, char* input, char** tokenInput) {
	TOKEN_SIZE = 0;
	memset(input, 0, sizeof(*input));
	memset(tokenInput, 0, sizeof(*tokenInput));
}

//History linked list에 추가해준다
void addHistory(char** tokenInput, int TOKEN_SIZE) {
	History *temp = (History*)calloc(sizeof(History), 1);
	strcpy(temp->command, tokenInput[0]);
	switch (TOKEN_SIZE) {
	case 2:
		strcat(temp->command, " ");//형식에 맞춰 명령어" "명령어", "명령어"".. 형식으로 출력
		strcat(temp->command, tokenInput[1]);
		break;
	case 3:
		strcat(temp->command, " ");
		strcat(temp->command, tokenInput[1]);
		strcat(temp->command, ", ");
		strcat(temp->command, tokenInput[2]);
		break;
	case 4:
		strcat(temp->command, " ");
		strcat(temp->command, tokenInput[1]);
		strcat(temp->command, ", ");
		strcat(temp->command, tokenInput[2]);
		strcat(temp->command, ", ");
		strcat(temp->command, tokenInput[3]);
		break;
	}
	if (!historyHead) {
		historyHead = temp; //historyEnd = temp;//만약에 아직 history list가 empty라면
		historyEnd =temp;
	}
	else {
		historyEnd->link = temp;
		historyEnd = temp;//history list가 있을 경우 끝에 추가
	}
}
//error메세지 출력
void printError(int num) {
	if (num == 1) printf("Error : Invalid command\n");
	else if (num == 2) printf("Error : Invalid Argument\n");
	else if (num == 3) printf("Error : Out of Range \n");
	else if (num == 4) printf("Error : Invalid Address Range\n");
	else if (num == 5) printf("Error : Invalid Mnemonic.\n");
	else if (num == 6) printf("Error : Invalid File name\n");
	else if (num == 7) printf("Error : There is no start address\n");
	else if (num == 8) printf("Error : It is invalid Value\n");
}

//16진수인지 체크해주는 함수
bool chkHexa(char x) {
	if (('0' <= x && x <= '9') || ('a' <= x && x <= 'f') || ('A' <= x && x <= 'F'))return true;
	else return false;
}

bool isHexa(char* s) {
	int i;
	for (i = 0; i < strlen(s); i++) {
		if (chkHexa(s[i])) continue;
		else return false;
	}
	return true;
}

//명령이 올바른 format인지 체크해줌
//TOKEN 개수는 최대 4개이며 모두 [명령여][공백][숫자][, ][숫자][, ][숫자]의 형식을 가지고있다
//따라서 input을 끝까지 돌면서 형식이 맞는지 체크해준다
bool ChkRightFormat(char* input, int TOKEN_SIZE) {
	int i;
	char inputTmp[INPUT_SIZE];
	strcpy(inputTmp, input);
	if (TOKEN_SIZE == 1) return true;
	if (TOKEN_SIZE == 2) {
		i = 0;
		while (inputTmp[i] != ' ') i++;
		i++;
		while (i != strlen(inputTmp)) {
			if (!chkHexa(inputTmp[i])) {
				return false;
			}
			i++;
		}
		return true;
	}
	if (TOKEN_SIZE == 3) {
		i = 0;
		while (inputTmp[i] != ',') i++;
		i++;
		if (inputTmp[i] != ' ') { return false; }
		i++;
		while (i != strlen(inputTmp)) {
			if (!chkHexa(inputTmp[i])) {
				return false;
			}
			i++;
		}
		return true;
	}
	if (TOKEN_SIZE == 4) {
		return true;
	}
	else {
		return false;
	}
}
void checkCommand(char* input, int TOKEN_SIZE, char** tokenInput, History* historyHead, History* historyEnd, int lastAddr, int* memory, int* flag, Hashnode** hashtable) {
	if (TOKEN_SIZE == 1) {
		if (STRCMP_("h") || STRCMP_("help")) {addHistory(tokenInput, TOKEN_SIZE); printfunctionHelp(); }
		else if (STRCMP_("d") || STRCMP_("dir")) { addHistory(tokenInput, TOKEN_SIZE); functionDir(); }
		else if (STRCMP_("q") || STRCMP_("quit")) { *flag = 1; }
		else if (STRCMP_("hi") || STRCMP_("history")) { addHistory(tokenInput, TOKEN_SIZE);functionHistory(historyHead); }
		else if (STRCMP_("du") || STRCMP_("dump")) { addHistory(tokenInput, TOKEN_SIZE); functionDump(TOKEN_SIZE, tokenInput, memory, &lastAddr); }
		else if (STRCMP_("reset")) { addHistory(tokenInput, TOKEN_SIZE); resetMemory(memory); }
		else if (STRCMP_("opcodelist")) { printOpcode(hashtable); }
		else if (STRCMP_("symbol")) {
			if (assembleFlag==true) printSymTable();
			else printf("[ERROR] : Assemble instruction should come first\n");
		}
		else printError(1);
	}
	else if (STRCMP_("du") || STRCMP_("dump")) { if (TOKEN_SIZE <= 3 && ChkRightFormat(input, TOKEN_SIZE)) { if (functionDump(TOKEN_SIZE, tokenInput, memory, &lastAddr)) addHistory(tokenInput, TOKEN_SIZE); } else printError(1); }
	else if (STRCMP_("e") || STRCMP_("edit")) { if (TOKEN_SIZE == 3 && ChkRightFormat(input, TOKEN_SIZE)) { if (functionEdit(tokenInput, memory))addHistory(tokenInput, TOKEN_SIZE); } else printError(1); }
	else if (STRCMP_("f") || STRCMP_("fill")) { if (TOKEN_SIZE == 4 && ChkRightFormat(input, TOKEN_SIZE)) { if (functionFill(tokenInput, memory) == true) addHistory(tokenInput, TOKEN_SIZE); } else  printError(1); }
	else if (STRCMP_("opcode")) { if (ChkRightFormatOpcode(input))printMnemonic(tokenInput, hashtable, TOKEN_SIZE, historyHead, historyEnd); }
	else if ((STRCMP_("type") && TOKEN_SIZE == 2)) { typeFilename(tokenInput, historyHead, historyEnd); }
	else if (STRCMP_("assemble")) {
		pass1(tokenInput, hashtable, historyHead, historyEnd); 
		assembleFlag=true;
	}
	else printError(1);
}

int main(void) {
	int memory[MEMORY_SIZE] = { 0 };
	char input[INPUT_SIZE];
	char** tokenInput;//tokenized input will be stored
	int TOKEN_SIZE, lastAddr = 0;
	int flag = 0; //program ends when flag is true
	Hashnode *hashtable[20];
	//make OPCODE_LIST
	makeHashTable(hashtable);
	while (1) {
		printf("sicsim>");
		fgets(input, sizeof(input), stdin);
		input[strlen(input) - 1] = '\0';
		tokenInput = (char**)calloc(sizeof(char*)*(INPUT_SIZE + 1), 1);
		if (tokenize(&TOKEN_SIZE, input, tokenInput) == true) checkCommand(input, TOKEN_SIZE, tokenInput, historyHead, historyEnd, lastAddr, memory, &flag, hashtable);
		//if flag is true, quit function is triggered
		if (flag == 1 && (STRCMP_("q") || STRCMP_("quit"))) break;
		setMemory(TOKEN_SIZE, input, tokenInput);
	}
	return 0;
}