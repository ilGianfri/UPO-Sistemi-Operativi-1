#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int seminit(int semid, int semnum, int initval);
	/* inizializzazione con initval della componente semnum di semid */

int down(int semid, int semnum);
	/* operazione DOWN sulla componente semnum di semid */

int up(int semid, int semnum);
	/* operazione UP sulla componente semnum di semid */



