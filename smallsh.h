#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define EOL 1 		/* end of line */
#define ARG 2		/* argomento normale */
#define AMPERSAND 3 	/* & */
#define SEMICOLON 4	/* ; */

#define MAXARG 512	/* numero massimo di argomenti */
#define MAXBUF 512	/* lunghezza massima riga di input */

#define	FOREGROUND 0
#define	BACKGROUND 1

int inarg(char c);		/* verifica se c non e' un carattere speciale */

int userin(char *p);		/* stampa il prompt e legge una riga */	

int gettok(char **outptr);	/* legge un simbolo */

int procline();			/* tratta una riga di input */

void runcommand(char **cline,int where);	/* esegue un comando */

