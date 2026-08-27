main: main.c Compiler.c processor.c memory.c
	gcc -g -c main.c
	gcc -g -c Compiler.c
	gcc -g -c processor.c
	gcc -g -c memory.c
	gcc -g main.o Compiler.o processor.o memory.o -o main

run:
	./main

clean:
	rm -f main.o
	rm -f Compiler.o
	rm -f processor.o
	rm -f memory.o
	rm -f main

debug:
	gdb -tui ./main 

.PHONY: run clean