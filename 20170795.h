#ifndef MAIN_H__
#define MAIN_H__
#define _CRT_SECIRE_NO_WARNINGS
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define INPUT_SIZE 0xff
#define MEMORY_SIZE 0xfffff
#define STRCMP_(x) (!strcmp(tokenInput[0],(x)))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

struct symbol* symHead;
struct symbol* symEnd;

struct obj* objHead;
struct obj* objEnd;

typedef struct _History {
	char command[INPUT_SIZE];
	struct _History* link;
}History;

typedef struct _Hashnode {
	char num[4];
	char mnemonic[8];
	char type[4];
	struct _Hashnode* link;
}Hashnode;

bool tokenize(int *TOKEN_SIZE, char* input, char** tokenInput);
void setMemory(int TOKEN_SIZE, char* input, char** tokenInput);
void addHistory(char** tokenInput, int TOKEN_SIZE);
void printError(int num);
bool chkHexa(char x);
bool isHexa(char* x);
bool ChkRightFormat(char* input, int TOKEN_SIZE);
void checkCommand(char* input, int TOKEN_SIZE, char** tokenInput, History* historyHead, History* historyEnd, int lastAddr, int* memory, int* flag, Hashnode** hashtable);
#endif