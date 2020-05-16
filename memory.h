#ifndef MEMORY_H__
#define MEMORY_H__

bool functionDump(int TOKEN_SIZE, char** tokenInput, int* memory, int* lastAddr);
bool functionEdit(char** tokenInput, int* memory);
bool functionFill(char** tokenInput, int* memory);
void resetMemory(int* memory);

#endif