build:
	gcc -ansi -c 6502.c memory.c

test:
	gcc -ansi 6502.c memory.c tests.c -o tests.o
	./tests.o

rom:
	gcc -ansi rom.c -o memory.rom

run:
	./6502.o