smallsh: smallsh.o input.o
	gcc -o smallsh -Wall -pedantic input.o smallsh.o 
smallsh.o: smallsh.c smallsh.h
	gcc -c -Wall -pedantic smallsh.c
input.o: input.c smallsh.h
	gcc -c -Wall -pedantic input.c
clean:
	rm smallsh input.o smallsh.o
