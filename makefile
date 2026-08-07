main: main.c Compiler.c processor.c memory.c
	gcc -c main.c
	gcc -c Compiler.c
	gcc -c processor.c
	gcc -c memory.c
	gcc main.o Compiler.o processor.o memory.o -o main

	./main
	rm -f main.o
	rm -f Compiler.o
	rm -f processor.o
	rm -f memory.o

.PHONY: run clean