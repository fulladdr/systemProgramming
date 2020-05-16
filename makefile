20170795.out : 20170795.o shell.o memory.o opcode.o assembler.o
	gcc -o 20170795.out 20170795.o shell.o memory.o opcode.o assembler.o

20170795.o : 20170795.c 20170795.h
	gcc -c 20170795.c -Wall

shell.o : shell.c shell.h
	gcc -c shell.c -Wall

memory.o : memory.c memory.h
	gcc -c memory.c -Wall

opcode.o : opcode.c opcode.h
	gcc -c opcode.c -Wall

assembler.o : assembler.c assembler.h
	gcc -c assembler.c -Wall

clean :
	rm 20170795.o shell.o memory.o opcode.o assembler.o
