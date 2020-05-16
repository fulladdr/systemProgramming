#include "20170795.h"
#include "assembler.h"
#include "opcode.h"

const char* directiveList[8] = { "START", "END", "BYTE", "WORD", "RESB", "RESW", "BASE", "NOBASE" };
const char* registerList[9] = { "A","X","L","PC","SW","B","S","T","F" };

void makeObj(int location, int type, int total, int x, int disp, char opcode[LEN]) {//object code를 linked list 형태로 연결
	struct obj* tmp = (struct obj*)calloc(sizeof(struct obj), 1);
	tmp->location = location;//lcoation 값
	tmp->type = type;//형식 값
	tmp->total = total;
	tmp->x = x;
	tmp->disp = disp;//displacement 값
	strcpy(tmp->opcode, opcode);
	//linked list 형태로 연결해준다
	if (!objHead) {
		objHead = tmp;
		objEnd = tmp;
	}
	else {
		objEnd->ptr = tmp;
		objEnd = tmp;
	}
}
//레지스터 값을 반환한다
int registerNum(char* reg) {
	if (!strcmp(reg, "A")) return 0;
	if (!strcmp(reg, "X")) return 1;
	if (!strcmp(reg, "L")) return 2;
	if (!strcmp(reg, "B")) return 3;
	if (!strcmp(reg, "S")) return 4;
	if (!strcmp(reg, "T")) return 5;
	if (!strcmp(reg, "F")) return 6;
	if (!strcmp(reg, "PC")) return 8;
	if (!strcmp(reg, "SW")) return 9;
	else return -1;
}

//symbol table에 statement이 있는지 체크해준다
int chkInSym(char* statement) {
	struct symbol* tmp = symHead;
	while (tmp) {
		if (!strcmp(tmp->symbol, statement)) return tmp->location;//같다면 location 값을 반환함
		else tmp = tmp->ptr;
	}
	return -1;
}
//linked list 형태로 symbol table을 만듬
int makeSymbolTable(char* statement, int loc) {
	struct symbol* s;
	s = symHead;
	while(s){
		if (!strcmp(s->symbol, statement)){//이미 존재한다면 0을  리턴함
			return 0;
		}
		s = s->ptr;
	}
	struct symbol* tmp = (struct symbol*)calloc(sizeof(struct symbol), 1);
	tmp->location = loc; 
	strcpy(tmp->symbol, statement);
	if (symHead == NULL) {//첫번재 노드라면 symHead로 지정해준다
		symHead = tmp; symEnd = tmp;
	}
	else {
		symEnd->ptr = tmp;//list끝에 값을 연결한다
		symEnd = tmp;
	}
	return 1;
}
//Opcode.txt에 있는 statement인지 확인해준다
//Hash값을 계산해서 해당 table[hashNum]에 statement가 있는 확인
info* isInHash(Hashnode** table, char* statement) {
	info* i = (info*)calloc(sizeof(info), 1);
	char* tmp = (char*)calloc(sizeof(char), strlen(statement));
	if (statement[0] == '+') {//4형식일 경우 +가 없는 채로 opcode에서 찾음
		strcpy(tmp, statement + 1);
	}
	else strcpy(tmp, statement);
	int num = makeHashIndex(tmp);//해쉬인덱스를 구함
	int a;
	Hashnode* ptr = table[num];
	while (ptr) {
		if (!strcmp(tmp, ptr->mnemonic)) {
			sscanf(ptr->num, "%x", &a);
			strcpy(i->type, ptr->type);
			i->opcode = a;
			return i;//존재한다면 구조체에 저장해 리턴한다
		}
		ptr = ptr->link;
	}
	return i;
}

//head node와 비교하여 더 작으면 왼쪽에 연결
//head node와 비교하여 더 크면 오른족에 연결
int printSymTable(void) {//연결리스트로 symbol table 구현
struct symTable* head = NULL;
struct symbol* tmp = symHead;
	while (tmp) {
		struct symTable* node = (struct symTable*)calloc(sizeof(struct symTable), 1);
		strcpy(node->symbol, tmp->symbol);
		node->location = tmp->location;
		if (head == NULL) {
			head = node;
		}
		else {
			if (strcmp(tmp->symbol, head->symbol) <0) {
				while (head->front != NULL && strcmp(tmp->symbol, head->symbol)<0) {//더 작은 값까지
					head = head->front;
				}
				if (head->front == NULL && strcmp(tmp->symbol, head->symbol)<0) {//만약에 마지막 값보다 작다면 끝에 연결
					node->back = head;
					head->front = node;
				}
				else {//노드 연결
					node->front = head;
					node->back = head->back;
					head->back->front = node;
					head->back = node;
				}
			}
			else if (strcmp(tmp->symbol, head->symbol)>0) {//더 큰 값까지 
				while (head->back != NULL && strcmp(tmp->symbol, head->symbol)>0) {
					head = head->back;
				}
				if (head->back == NULL && strcmp(tmp->symbol, head->symbol) >1) {//만약에 마지막 값보다 크다면 끝에 연결
					node->front = head;
					head->back = node;
				}
				else {
					node->back = head;
					node->front = head->front;
					head->front->back = node;
					head->front = node;
				}
			}
		}
		tmp = tmp->ptr;
	}
	struct symTable* ptr = head;
	while (ptr->front != NULL) {//테이블의 가장 앞값까지 이동
		ptr = ptr->front;
	}
	while (ptr != NULL) {//NULL값을 가리킬때까지 ptr을 이동시켜준다
		fprintf(stdout, "%s\t%04X\n", ptr->symbol, ptr->location);
		ptr = ptr->back;
	}
	return 0;
}
//pass2 구현
int pass2(char** tokenInput, char* midfile, char* lstfile, char* objfile, Hashnode** table) {
	FILE* mid; FILE* lst; FILE* obj;
	bool startFlag = false, baseFlag = false, endFlag = false, existSymbol = false, isNum = false;
	int lineNum, location, disp, pc, base = 0;
	int nixbpe[6] = { 0 };
	char symbol[LEN], oper[LEN], op1[LEN], op2[LEN], programName[LEN], tmp[501];
	//make listing file
	mid = fopen(midfile, "rt");
	lst = fopen(lstfile, "wt");
	if (!lst){
		printf("[ERROR] : There is no corresponding file\n");
		return 0;
	}
	while (fgets(tmp, sizeof(tmp), mid)) {
		existSymbol = false;
		disp = 0;
		if (tmp[0] == '.') {//주석일 경우
			char str[501];
			strcpy(str, tmp + 1);
			fprintf(lst, "%s", str);
			continue;
		}
		//initialize
		strcpy(symbol, "");
		strcpy(oper, "");
		strcpy(op1, "");
		strcpy(op2, "");
		lineNum = 0, location = 0;
		sscanf(tmp, "%d %x %s %s %s %s", &lineNum, &location, symbol, oper, op1, op2);
		if (!strcmp(symbol, "<")) strcpy(symbol, "");//< 심볼이 들어갔다면 값이 존재하지 않는다는 것이다
		if (!strcmp(oper, "<")) strcpy(oper, "");
		if (!strcmp(op1, "<")) strcpy(op1, "");
		if (!strcmp(op2, "<")) strcpy(op2, "");
		//object code 계산할 필요 없는 경우
		if (!strcmp(oper, "START")) {
			if (strcmp(symbol, ""))strcpy(programName, symbol);//obj file생성 위해 이름 넣어둠
			else strcpy(programName, "NONAME");//이름이 없을 경우 noname 출력
			startFlag = true;//start 시그널이 들어왔음을 알림
			fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			continue;
		}
		if (!strcmp(oper, "END")) {
			if (!strcmp(op1, "")){
				printf("[ERROR] : Line %d has no operand in END symbol\n", lineNum);
				return 0;
			}
			fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			endFlag = true; break;//end 시그널이 들어왔음을 알림
		}
		if (!strcmp(oper, "NOBASE")) { baseFlag = 0; continue; }
		if (!strcmp(oper, "BASE")) {
			baseFlag = true;
			struct symbol* tmp = symHead;
			while (tmp) {
				if (!strcmp(tmp->symbol, op1)) {
					base = tmp->location;//base location값을 받아놓는다
					break;
				}
				else tmp = tmp->ptr;
			}
			fprintf(lst, "%4d    %-8s\t%-8s\t%-8s\t%-8s\n", lineNum, symbol, oper, op1, op2);
			continue;
		}
		if (!strcmp(oper, "RESB") || !strcmp(oper, "RESW")) {//변수인 경우
			fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			makeObj(location, 8, 0, 0, 0, "");
			continue;
		}
		if (!strcmp(oper, "BYTE")) {
			if (op1[0] == 'C') {
				char* str = (char*)calloc(sizeof(char), (strlen(op1) + strlen(op2) + 2));
				strcat(str, op1);
				strcat(str, "'");
				strcat(str, op2);
				strcat(str, "'");
				fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t        ", lineNum, location, symbol, oper, str);//형식을 위해 str로 합친 후 프린트해줌
				for (int i = 0; i < strlen(op2); i++) {
					fprintf(lst, "%x", op2[i]);
				}
				makeObj(location, 6, 0, 0, 0, op2);//type 6로 object code로 기록함
				fprintf(lst, "\n");
			}
			else if (op1[0] == 'X') {
				char* str = (char*)calloc(sizeof(char), (strlen(op1) + strlen(op2) + 2));
				strcat(str, op1);
				strcat(str, "'");
				strcat(str, op2);
				strcat(str, "'");
				fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t        ", lineNum, location, symbol, oper, str);
				for (int i = 0; i < strlen(op2); i++) {
					fprintf(lst, "%c", op2[i]);
				}
				fprintf(lst, "\n");
				makeObj(location, 7, 0, 0, 0, op2);
			}
			continue;
		}
		//object code 계산해야하는 경우
		info* obj = isInHash(table, oper);
		int opcode = obj->opcode;//opcode 번호
		char* type = obj->type;//형식
		if (!obj) {
			fprintf(stdout, "[ERROR] : There is unvalid mnemonic\n");
			fclose(lst);
			remove(lstfile);
			return 0;
		}
		//2형식 [OPCODE][reg1][reg2]
		else if (!strcmp(type, "2")) {
			int reg1, reg2;
			//reg1만 주어진 경우
			if (!strcmp(op2, "")) {
				fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s", lineNum, location, symbol, oper, op1, op2);
				if (!strcmp(oper, "SVC")) reg1 = atoi(op1);
				else reg1 = registerNum(op1);//레지스터 값을 읽어옴
				int x = opcode;
				fprintf(lst, "%02X%01X0\n", x, reg1);
				makeObj(location, 4, x, reg1, 0, "");
			}
			//reg1 and reg 2
			else {
				char* str = (char*)calloc(sizeof(char), strlen(op1) + strlen(op2) + 1);
				strcat(str, op1);
				strcat(str, ",");
				strcat(str, op2);
				fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t        ", lineNum, location, symbol, oper, str);
				reg1 = registerNum(op1);
				reg2 = registerNum(op2);
				int x = opcode;
				if (!strcmp(oper, "SHIFTR") || !strcmp(oper, "SHIFTL")) { reg2 = atoi(op2); }
				fprintf(lst, "%02X%01X%01X\n", x, reg1, reg2);
				makeObj(location, 5, x, reg1, reg2, "");
			}
			pc = location + 2;//program counter값을 조정함
			continue;
		}
		//1형식 [OPCODE]
		if (strcmp(op2, "")) {
			char* str = (char*)calloc(sizeof(char), strlen(op1) + strlen(op2) + 1);
			strcat(str, op1);
			strcat(str, ",");
			strcat(str, op2);
			fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t        ", lineNum, location, symbol, oper, str);
		}
		else fprintf(lst, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s", lineNum, location, symbol, oper, op1, op2);
		if (!strcmp(type, "1")) {
			fprintf(lst, "%02X\n", opcode);
			makeObj(location, 3, opcode, 0, 0, "");
			pc = location + 1;
		}
		else if (!strcmp(type, "3/4")) {
			if (oper[0] == '+') {
				pc = location + 4;
				strcpy(type, "4");//+ sign이 있으면 4형식
			}
			//[OPCODE][nixbpe][disp]
			else {
				strcpy(type, "3");
				if (!strcmp(oper, "RSUB")) {
					fprintf(lst, "4F0000\n");
					makeObj(location, 2, 0, 0, 0, "4F0000");
					continue;
				}
				pc = location + 3;
			}
			char tmpOper[101];
			strcpy(tmpOper, op1);
			//immediate addreessing
			if (op1[0] == '#') {
				nixbpe[0] = 0; nixbpe[1] = 1;
				strcpy(tmpOper, op1 + 1);
			}
			else if (op1[0] == '@') {
				nixbpe[0] = 1; nixbpe[1] = 0;
				strcpy(tmpOper, op1 + 1);
			}
			else {
				nixbpe[0] = 1; nixbpe[1] = 1;
			}
			for (int i = 0; i < strlen(tmpOper); i++) {
				if ('0' <= tmpOper[i] && tmpOper[i] <= '9') continue;//상수만으로 이루어졌는지 확인함
				else { isNum = true; break; }
			}
			//op1 값이 상수일 경우
			if (isNum == false) {
				disp = atoi(tmpOper);//실수이다
				if (!strcmp(type, "4") && disp >= 0x100000) {//4형식인 경우
					fprintf(stdout, "[ERROR] : Line %d op1 range overflow\n", lineNum);
					fclose(lst);
					remove(lstfile);
					return 0;
				}
				else if (!strcmp(type, "3") && disp >= 0x1000) {//3형식인 경우
					fprintf(stdout, "[ERROR] : Line %d op1 range overflow\n", lineNum);
					fclose(lst);
					remove(lstfile);
					return 0;
				}
				else {
					if (!strcmp(type, "3")) {//nixbpe 값 지정
						nixbpe[2] = 0; nixbpe[3] = 0; nixbpe[4] = 0; nixbpe[5] = 0;
					}
					else if (!strcmp(type, "4")) {
						nixbpe[2] = 0; nixbpe[3] = 0; nixbpe[4] = 0; nixbpe[5] = 1;
					}
				}
			}
			else {
				int symLoc;
				struct symbol* tmp = symHead;
				while (tmp) {
					if (!strcmp(tmp->symbol, tmpOper)) {
						symLoc = tmp->location;//location값을 받아놓는다
						existSymbol = true;
						break;
					}
					else tmp = tmp->ptr;
				}
				if (existSymbol == false) {
					fprintf(stdout, "[ERROR] : Line %d has wrong symbol pointer\n", lineNum);
					fclose(lst);
					remove(lstfile);
					return 0;
				}
				else {//symbol Exist
					if (!strcmp(type, "4")) {
						disp = symLoc;
						nixbpe[2] = 0; nixbpe[3] = 0; nixbpe[4] = 0; nixbpe[5] = 1;
					}
					else if (!strcmp(type, "3")) {
						disp = symLoc - pc;
						if (-0x800 <= disp && disp <= -1) {
							nixbpe[2] = 0; nixbpe[3] = 0; nixbpe[4] = 1; nixbpe[5] = 0;
							disp += 0x1000;
						}
						else if (-0x800 <= disp && disp <= 0x7FFF) {
							nixbpe[2] = 0; nixbpe[3] = 0; nixbpe[4] = 1; nixbpe[5] = 0;
						}
						else {
							if (baseFlag == false) {
								fprintf(stdout, "[ERROR] : Line %d Base Flag is not settled\n", lineNum);
								fclose(lst);
								remove(lstfile);
								return 0;
							}
							tmp = symHead;
							int lc;
							bool chk = false;
							while (tmp) {//symboil table에 값이 존재하는지 체크함
								if (!strcmp(tmp->symbol, op1)) {
									lc = tmp->location;//location값을 받아놓는다
									chk = true;
									break;
								}
								else tmp = tmp->ptr;
							}
							if (chk == false) {
								fprintf(stdout, "[ERROR] : Line %d op1 is not in the symbol table\n", lineNum);
								fclose(lst);
								remove(lstfile);
								return 0;
							}
							disp = lc - base;
							if (disp >= 0 && disp <= 0xFFF) {
								nixbpe[2] = 0; nixbpe[3] = 1; nixbpe[4] = 0; nixbpe[5] = 0;
							}
							else {
								fprintf(stdout, "[ERROR] : Line %d disp range error\n", lineNum);
								fclose(lst);
								remove(lstfile);
								return 0;
							}
						}
					}
				}
			}
			//object 코드 만듬
			if (!strcmp(op2, "X") || !strcmp(op2, "x")) {
				nixbpe[2] = 1;//x값을 조정해준다
			}
			if (!strcmp(type, "3")) {//object code 조정
				int tmp, n, i, total, x;
				tmp = opcode;
				n = nixbpe[0] * 2;
				i = nixbpe[1];
				total = tmp + n + i;
				x = nixbpe[2] * 8 + nixbpe[3] * 4 + nixbpe[4] * 2 + nixbpe[5];
				fprintf(lst, "%02X%01X%03X\n", total, x, disp);
				makeObj(location, 1, total, x, disp, "");
			}
			else if (!strcmp(type, "4")) {
				int tmp, n, i, total, x;
				tmp = opcode;
				n = nixbpe[0] * 2;
				i = nixbpe[1];
				total = tmp + n + i;
				x = nixbpe[2] * 8 + nixbpe[3] * 4 + nixbpe[4] * 2 + nixbpe[5];
				fprintf(lst, "%02X%01X%05X\n", total, x, disp);
				if (existSymbol == true)makeObj(location, 9, total, x, disp, "");
				else makeObj(location, 9, total, x, disp, "z");
			}
			isNum = false;
			existSymbol = false;
		}
	}
	if (startFlag == false || endFlag == false) {//start flag나 end flag가 없을 경우 에러 출력
		fprintf(stdout, "[ERROR] : Start/End Signal is not settled\n");
		fclose(lst);
		remove(lstfile);
		return 0;
	}
	//make object file
	obj = fopen(objfile, "wt");
	if (!obj) {
		fprintf(stdout, "[ERROR] : Obj file cannot be opened\n");
		return 0;
	}
	//header 부분 작성
	fprintf(obj, "H%-6s000000%06X\n", programName, location);
	//T부분 작성
	struct obj* tmp1;
	struct obj* tmp2;
	tmp1 = objHead;
	tmp2 = objHead;
	while (tmp1 != NULL) {
		int len = 0;
		while (tmp1->type == 8) tmp1 = tmp1->ptr;
		fprintf(obj, "T%06X", tmp1->location);
		while (tmp2) {
			if (tmp2->type == 8) {
				while (tmp2->type == 8) {
					tmp2 = tmp2->ptr;
				}
				break;
			}
			else if (tmp2->type == 7) {
				if ((len + strlen(tmp2->opcode) / 2) > 30) break;
				len += strlen(tmp2->opcode) / 2;
			}
			else if (tmp2->type == 6) {
				if ((len + strlen(tmp2->opcode)) > 30) break;
				len += strlen(tmp2->opcode);
			}
			else if (tmp2->type == 3) {
				if (len + 1 > 30) break;
				len += 1;
			}
			else if (tmp2->type == 4 || tmp2->type == 5) {
				if (len + 2 > 30)break;
				len += 2;
			}
			else if (tmp2->type == 9) {//4형식인 경우
				if (len + 4 > 30)break;
				len += 4;
			}
			else {//일반적인 3형식
				if (len + 3 > 30) break;
				len += 3;
			}
			if (len > 30)break;//라인길이를 30으로 지정함
			else tmp2 = tmp2->ptr;
		}
		if (len != 0)fprintf(obj, "%02X", len);
		while (tmp1 != tmp2) {
			if (tmp1->type == 1) fprintf(obj, "%02X%01X%03X", tmp1->total, tmp1->x, tmp1->disp);
			else if (tmp1->type == 2)fprintf(obj, "4F0000");
			else if (tmp1->type == 3)fprintf(obj, "%02X", tmp1->total);
			else if (tmp1->type == 4)fprintf(obj, "%02X%01X0", tmp1->total, tmp1->x);
			else if (tmp1->type == 5)fprintf(obj, "%02X%01X%01X", tmp1->total, tmp1->x, tmp1->disp);
			else if (tmp1->type == 6) {
				for (int i = 0; i < strlen(tmp1->opcode); i++) {
					fprintf(obj, "%X", tmp1->opcode[i]);
				}
			}
			else if (tmp1->type == 7) {
				for (int i = 0; i < strlen(tmp1->opcode); i++) {
					fprintf(obj, "%C", tmp1->opcode[i]);
				}
			}
			else if (tmp1->type == 9) {
				fprintf(obj, "%02X%01X%05X", tmp1->total, tmp1->x, tmp1->disp);
			}
			tmp1 = tmp1->ptr;
		}
		fprintf(obj, "\n");
	}
	tmp1 = objHead;
	while (tmp1) {//4형식 값을 보정해준다
		if (tmp1->type == 9 && !strcmp(tmp1->opcode, "")) {
			fprintf(obj, "M%06X05\n", tmp1->location + 1);
		}
		tmp1 = tmp1->ptr;
	}
	fprintf(obj, "E000000");
	fprintf(stdout, "successfully assemble %s.\n", tokenInput[1]);
	fclose(obj);
	fclose(mid);
	fclose(lst);
	return 0;
}

int pass1(char** tokenInput, Hashnode** table, History* historyHead, History* historyEnd) {
	char midFile[LEN],asmFile[LEN], lstFile[LEN], objFile[LEN], temp[LEN];
	int location = 0, lineNum = 5;
	strcpy(midFile, "mid.txt");
	FILE* mid = fopen("mid.txt", "wt");
	FILE* fp = fopen(tokenInput[1], "rt");
	if (!fp || !mid) {
		fprintf(stdout, "[ERROR] File Open Error\n");
		return 0;
	}
	addHistory(tokenInput, 2);
	strcpy(asmFile, tokenInput[1]);
	//asm 파일이 맞는지 확인한다
	if (asmFile[strlen(asmFile) - 3] != 'a' || asmFile[strlen(asmFile) - 2] != 's' || asmFile[strlen(asmFile) - 1] != 'm') {
		fprintf(stdout, "[ERROR] File is not asm file\n");
		return 0;
	}
	free(objHead); free(objEnd); free(symEnd); free(symHead);
	objHead = (struct obj*)calloc(sizeof(struct obj), 1);
	symHead = (struct symbol*)calloc(sizeof(struct symbol), 1);
	objEnd = (struct obj*)calloc(sizeof(struct obj), 1);
	symEnd = (struct symbol*)calloc(sizeof(struct symbol), 1);
	symHead = symEnd = NULL;
	objHead = objEnd = NULL;
	strcpy(lstFile, tokenInput[1]);
	strcpy(objFile, tokenInput[1]);
	lstFile[strlen(lstFile) - 3] = 'l';	lstFile[strlen(lstFile) - 2] = 's';	lstFile[strlen(lstFile) - 1] = 't';
	objFile[strlen(objFile) - 3] = 'o';	objFile[strlen(objFile) - 2] = 'b';	objFile[strlen(objFile) - 1] = 'j';
	//tokenize asm file
	while (fgets(temp, sizeof(temp), fp)) {
		char symbol[LEN], oper[LEN], op1[LEN], op2[LEN];
		//empty string
		strcpy(symbol, "<");
		strcpy(oper, "<");
		strcpy(op1, "<");
		strcpy(op2, "<");
		struct statement *tmp = (struct statement*)calloc(sizeof(struct statement), 1);
		if (temp == NULL || temp[0] == '\n') continue;
		else if (temp[0] == '.') {//if it is just a comment
			fprintf(mid, ".%4d\t%s", lineNum, temp);//print immediatedly
			lineNum += 5; continue;
		}
		else if (temp[0] == '\t' || temp[0] == ' ') {//if it doesn't have a symbol
			tmp->type = 1;
			strcpy(tmp->stat, temp);
			char* ptr = strtok(temp, " ,\t\n");
			int n = 0;
			while (ptr != NULL) {
				tmp->token[n] = ptr;
				ptr = strtok(NULL, " ,\t\n'"); n++;//tokenize it
			}
			strcpy(oper, tmp->token[0]);
			if (tmp->token[1]) strcpy(op1, tmp->token[1]);
			if (tmp->token[2]) strcpy(op2, tmp->token[2]);
			tmp->num = n;
		}
		else {//if it has a symbol
			tmp->type = 2;
			strcpy(tmp->stat, temp);//copy original statement
			char* ptr = strtok(temp, " ,\t\n'");
			int n = 0;
			while (ptr != NULL) {
				tmp->token[n] = ptr;
				ptr = strtok(NULL, " ,\t\n'"); n++;
			}			
			strcpy(symbol, tmp->token[0]);
			strcpy(oper, tmp->token[1]);
			if (tmp->token[2]) strcpy(op1, tmp->token[2]);
			if (tmp->token[3]) strcpy(op2, tmp->token[3]);
			tmp->num = n;
		}
		//build intermediate file
		//object code 생성 전 중간 단계임
		if (strcmp(symbol, "<") && chkInSym(symbol) && strcmp(oper, "START")) {
			int x = makeSymbolTable(symbol, location);
			if (x==0){
				printf("[ERROR] : Line %d Error, There is already a symbol in symbol table\n", lineNum);
				return 0;
			}
		}
		if (!strcmp(oper, "START")) {
			int snum; 
			sscanf(op1, "%x", &snum);
			location = snum;
			fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			lineNum += 5;
		}
		else if (!strcmp(oper, "END")) {
			fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			break;//종료한다
		}
		else if (!strcmp(oper, "BYTE")) {
			if (!strcmp(op1, "C")) {//char일 경우
				fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
				location = location + strlen(op2);
			}
			else if (!strcmp(op1, "X")) {//16진수일경우
				if (!isHexa(op2)) {
					printf("[ERROR] : %d contains non hexadecimal number\n", lineNum);
					return 0;
				}
				fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
				location = location + (strlen(op2) / 2);

			}
			lineNum += 5;
		}
		else if (!strcmp(oper, "WORD")) {
			int num = atoi(op1);
			if (num > 0xFFFFFF) {
				printf("[ERROR] : %d contains overflow number\n", lineNum);
				return 0;
			}
			fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			location += 3;
			lineNum += 5;
		}
		else if (!strcmp(oper, "RESB")) {
			fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			location = location + atoi(op1);
			lineNum += 5;
		}
		else if (!strcmp(oper, "RESW")) {
			fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			location = location + 3 * atoi(op1);
			lineNum += 5;
		}
		else if (!strcmp(oper, "BASE")) {
			fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			lineNum += 5;
		}
		else if (!strcmp(oper, "NOBASE")) {
			fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
			lineNum += 5;
		}
		else {
			info* chk = isInHash(table, oper);
			//opcode.txt에 있는지 확인한다
			if (!chk) {
				printf("[ERROR] : %d is not in opcode.txt file\n", lineNum);//없으면 에러처리
				return 0;
			}
			else {
				if (strcmp(symbol, "")) fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
				else fprintf(mid, "%4d\t%04X\t%-8s\t%-8s\t%-8s\t%-8s\n", lineNum, location, symbol, oper, op1, op2);
				if (!strcmp(chk->type,"1"))location += 1;
				else if (!strcmp(chk->type, "2"))location += 2;
				else if (!strcmp(chk->type, "3/4") && oper[0] == '+') {
					location += 4;
				}
				else location += 3;
				lineNum += 5;
			}
		}
	}
	fclose(mid);
	pass2(tokenInput, midFile, lstFile, objFile, table);
	return 1;
}