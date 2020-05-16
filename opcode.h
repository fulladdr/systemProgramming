#ifndef OPCODE_H__
#define OPCODE_H__

#include "20170795.h"

int makeHashIndex(char *s);
Hashnode** makeHashTable(Hashnode** hashtable);
void printMnemonic(char** tokenInput, Hashnode** hashtable, int TOKEN_SIZE, History* historyHead, History* historyEnd);
void printOpcode(Hashnode** hashtable);
bool ChkRightFormatOpcode(char* input);
#endif