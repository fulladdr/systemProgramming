#include "20170795.h"
#include "shell.h"

void printfunctionHelp() {
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");
	printf("assemble filename\n");
	printf("type filename\n");
	printf("symbol\n");
}

bool functionDir() {
   DIR *dir = opendir("./");//read directory
   struct dirent* file; struct stat status;
   if (!dir) {
      fprintf(stderr, "directory open error\n");
      return false; //if it fails to read directory
   }
   int cnt = 0;
   while ((file = readdir(dir)) != NULL) {
      if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))continue;
      stat(file->d_name, &status);
      if (S_ISDIR(status.st_mode)) printf("%s/  ", file->d_name);//if its directory
      else if (S_IXUSR & status.st_mode) printf("%s*  ", file->d_name);//if its file
      else printf("%s  ", file->d_name); 
      cnt++;
      if (cnt % 4 == 0 ) printf("\n");
   }
   printf("\n");
   closedir(dir);
   return true;
}

void functionHistory(History* historyHead) {
	History* temp = historyHead;
	int start = 1;
	while (temp) {
		printf("%-6d %s\n", start, temp->command);
		start++; 
		temp = temp->link;
	}
}

void typeFilename(char** tokenInput,History* historyHead, History* historyEnd) {
	char* name = tokenInput[1];
	char tmp[255];
	FILE* fp = fopen(name, "r");
	if (!fp) {
		printf("There is no such file\n");
		return;
	}
	else addHistory(tokenInput, 2 );
	while (fgets(tmp, sizeof(tmp), fp)) {
		printf("%s", tmp);
	}
	printf("\n");
	fclose(fp);
}