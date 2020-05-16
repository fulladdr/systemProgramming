#ifndef ASSEMBLER_H__
#define ASSEMBLER_H__

#define LEN 101
#define STAT_LEN 101
#define SYM_LEN 1001
#define OPLEN 21

//hash table에서 검색 후 정보 반환형
typedef struct _info {
	int opcode;//opcode
	char type[4];//형식 구분자
}info;

//asm file의 statement를 tokenize한 구조체
struct statement {
	int type;//statement type, 1: no symbol, 2: symbol
	int num;//token의 개수	
	char stat[STAT_LEN];//전체 statement
	char* token[LEN];//token화한 statement
};

//used to make temporary symbol table
struct symbol {
	int location;
	char symbol[LEN];
	struct symbol* ptr;//to insert as a linked list
};

//used to make symbol table
struct symTable {
	int location;
	char symbol[LEN];
	struct symTable* back;
	struct symTable* front;
};

//used to make object code
struct obj {
	int location;
	int type;
	int total;
	int x;
	int disp;
	char opcode[LEN];
	struct obj* ptr;//그다음 object code와 연결
};


void makeObj(int location, int type, int total, int x, int disp, char opcode[LEN]);
int registerNum(char* reg);
int chkInSym(char* statement);
int makeSymbolTable(char* statement, int loc);
info* isInHash(Hashnode** table, char* statement);
int printSymTable(void);
int pass2(char** tokenInput, char* midfile, char* lstfile, char* objfile, Hashnode** table);
int pass1(char** tokenInput, Hashnode** table, History* historyHead, History* historyEnd);

#endif