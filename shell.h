#ifndef SHELL_H__
#define SHELL_H__

void printfunctionHelp(void);
bool functionDir(void);
void functionHistory(History* historyHead);
void typeFilename(char** tokenInput,History* historyHead, History* historyEnd);

#endif