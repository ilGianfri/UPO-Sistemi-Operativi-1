#include "smallsh.h"
#include <sys/types.h>

char *prompt;
pid_t pid_foreground;

int procline(void)        /* tratta una riga di input */
{
  char *arg[MAXARG + 1];  /* array di puntatori per runcommand */
  int toktype;            /* tipo del simbolo nel comando */
  int narg;               /* numero di argomenti considerati finora */
  int type;               /* FOREGROUND o BACKGROUND */

  narg = 0;

  while (1)
  { /* ciclo da cui si esce con il return */

    /* esegue un'azione a seconda del tipo di simbolo */

    /* mette un simbolo in arg[narg] */

    switch (toktype = gettok(&arg[narg]))
    {

    /* se argomento: passa al prossimo simbolo */
    case ARG:

      if (narg < MAXARG)
        narg++;
      break;
    
    /* se fine riga o ';' o '&' esegue il comando ora contenuto in arg,
	 mettendo NULL per segnalare la fine degli argomenti: serve a execvp */
    case EOL:
    case SEMICOLON:
    case AMPERSAND:
      type = (toktype == AMPERSAND) ? BACKGROUND : FOREGROUND;

      if (narg != 0)
      {
        arg[narg] = NULL;
        runcommand(arg, type);
      }

      /* se fine riga, procline e' finita */
      if (toktype == EOL)
      {
        /* Notifica dei processi in background terminati */
        processEndNotifier();
        return 1;
      }

      /* altrimenti (caso del comando terminato da ';' o '&') 
           bisogna ricominciare a riempire arg dall'indice 0 */

      narg = 0;
      break;
    }
  }
}

//Punto 2
void processEndNotifier()
{
  int exitcode = 0;
  pid_t pid;

  /*
  WAITPID ritorna 0 se usato con WNOHANG e non ci sono figli che hanno terminato
          ritorna -1 in caso di errore
          altrimenti ritorna il pid del figlio

          Se usato senza WNOHANG sospende il padre finchè il figlio non termina

  WNOHANG controlla i processi figli senza sospendere il chiamante (padre)
  PID -1 controlla tutti i figli
  WEXITSTATUS riporta gli 8 bit meno significativi del codice di ritorno del figlio
  */
  while ((pid = waitpid(-1, &exitcode, WNOHANG)) > 0)
  {
    printf("Il processo %d è terminato con codice %d\n", pid, WEXITSTATUS(exitcode));
    bpid_remove(pid);
  }
}

//punto 3
void sig_handler(int sig)
{
  /* Se il segnale ricevuto è un SIGINT, lo ridireziona al processo in foreground */
  if (sig == SIGINT)
  {
    kill(pid_foreground, SIGINT);
    //pid_foreground = NULL;
  }

  /* Ridireziona la gestione del segnale all'azione di default */
  signal(SIGINT, SIG_DFL);
}

//punto 5
void bpid_add(pid_t pid)
{
  char *BPID = getenv("BPID");
  char *new = calloc(sizeof(BPID) + sizeof(pid) + 1, sizeof(char));

  strcpy(new, BPID);
  
  char *spid = calloc(sizeof(pid) + 1, sizeof(char));
  sprintf(spid, "%d", pid);
  
  strcat(new, ":");
  strcat(new, spid);
  
  if (new[0] == ':')
    memmove(new, new+1, strlen(new));

  setenv("BPID", new, 1);

  free(new);
  free(spid);
}

void bpid_remove(pid_t pid)
{
  char *BPID = getenv("BPID");
  char *ptr = strtok(BPID, ":");

  char *spid = (char *)calloc(sizeof(pid) + 1, sizeof(char));
  sprintf(spid, "%d", pid);

  /* Se c'è un solo PID non entra mai nel while perchè c'è un solo PID*/
  if (strchr(BPID, ':') == NULL)
    BPID = "";

  char *new = (char *)calloc(sizeof(BPID) + 1, sizeof(char));
  while (ptr != NULL)
  {
    if (strcmp(ptr, spid) != 0)
    {
      strcat(new, ":");
      strcat(new, ptr);
    }

    ptr = strtok(NULL, ":");
  }

  if (new[0] == ':')
      memmove(new, new+1, strlen(new));

  setenv("BPID", new, 1);

  free(ptr);
  free(new);
}

void runcommand(char **cline, int where) /* esegue un comando */
{
  if (strcmp(*cline, "bp") == 0)
  {
    printf("\nBPID: %s\n", getenv("BPID"));
    return;
  }

  pid_t pid;
  int exitstat, ret;

  pid = fork();
  if (pid == (pid_t)-1)
  {
    perror("smallsh: fork fallita");
    return;
  }

  if (pid == (pid_t)0)
  { /* processo figlio */

    /* esegue il comando il cui nome e' il primo elemento di cline,
       passando cline come vettore di argomenti */

    execvp(*cline, cline);
    perror(*cline);
    exit(1);
  }

  /* processo padre: avendo messo exec e exit non serve "else" */

  if (where == FOREGROUND)
  {
    /* Se il comando è in foreground salva il pid e ridireziona la gestione di SIGINT al metodo sig_handler */
    signal(SIGINT, sig_handler);
    pid_foreground = pid;

    ret = wait(&exitstat);
  }
  else
  {
    /* Se in background, gestione di default di SIGINT */
    signal(SIGINT, SIG_DFL);
    /* Aggiunge il pid del nuovo processo in background alla variabile d'ambiente BPID  */
    bpid_add(pid);
    printf("\nProcesso in background. PID: %d\n", pid);
  }

  if (ret == -1)
    perror("wait");
}

int main()
{
  /* Gestione standard del SIGINT */ 
  signal(SIGINT, SIG_DFL);

  /* Crea variabile d'ambiente vuota BPID */
  int bpidres = setenv("BPID", "", 1);
  if (bpidres != 0)
    perror("Impossibile creare variabile d'ambiente BPID");

  /* Alloca spazio per il prompt. calloc invece di malloc perchè viene anche settato a 0 */
  prompt = calloc(100, sizeof(char));

  /* Popola il prompt tramite variabili d'ambiente */
  sprintf(prompt, "%%%s:%s:", getenv("USER"), getenv("HOME"));
  while (userin(prompt) != EOF)
    procline();

  /* Alla chiusura dealloca il prompt */
  free(prompt);
}
