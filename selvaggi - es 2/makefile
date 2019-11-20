selvaggi: selvaggi.o semfun.o
	gcc -o selvaggi -Wall -pedantic semfun.o selvaggi.o 
selvaggi.o: selvaggi.c
	gcc -c -Wall -pedantic selvaggi.c
semfun.o: semfun.c semfun.h
	gcc -c -Wall -pedantic semfun.c
clean:
	rm selvaggi semfun.o selvaggi.o
