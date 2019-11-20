#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "semfun.h"
#include <sys/types.h>
#include <unistd.h>

int porzioni = 0;                   /* Tiene conto del numero di porzioni ancora in pentola */
int n_volte_riempie = 0;            /* Tiene conto del numero di volte che il cuoco ha riempito la pentola */
pthread_mutex_t mutex_porzioni;     /* Per accedere alla variabile porzioni */
sem_t vuoto;                        /* Indica pentola vuota */
sem_t pieno;                        /* Indica pentola piena */

int n_selvaggi = 0, n_porzioni = 0, n_giri = 0;

void cuoco();
void selvaggio();

int main(int argc, char *argv[])
{
    if (argc < 4) 
    {
        perror("Parametri non validi.\n Uso: ./selvaggi #n_selvaggi# #n porzioni# #n_giri#");
        return 1;
    }

    /* Salva i vari parametri passati nelle variabili */
    n_selvaggi = atoi(argv[1]);
    n_porzioni = atoi(argv[2]);
    n_giri = atoi(argv[3]);

    /*  
        Creazione dei semafori condivisi fra i processi

        IPC_PRIVATE usato come chiave garantisce che il vettore è nuovo e può essere 
        condiviso tra tutti i processi parenti (tramite id).

        Crea 3 semafori condivisi.
            - 1 per accedere alla variabile porzioni
            - 1 per indicare la pentola vuota
            - 1 per indicare la pentola piena

        0600 flag - permessi di lettura/scrittura al proprietario e nessun diritto agli altri

        Restituisce un "array" di semafori creati
    */

    pid_t semid = semget(IPC_PRIVATE, 3, 0600);
    if (semid == -1) 
    {
        perror("Errore semget\n");
        exit(1);
    }

    printf("ID semafori: %d\n",semid);    

    /* Imposta il primo semaforo a 0 */

    // seminit(semid,0,0);

    /* Inizializza il mutex in condizione unlocked */
    pthread_mutex_init(&mutex_porzioni, NULL);

    /*  
        Inizializzazione semafori vuoto e pieno come condivisi tra processi.
        vuoto è settato ad 1 poichè la pentola è vuota e di conseguenza pieno è settato a 0  
    */
    // sem_init(&vuoto, 1, 1);
    // sem_init(&pieno, 1, 0);

    /* Creazione processo cuoco */
    pid_t pidcuoco = fork();
    if (pidcuoco == (pid_t)-1)
    {
        perror("fork cuoco fallita");
        return 1;
    }
    else if (pidcuoco == (pid_t)0)
    {
        cuoco();
    }

    for (int i = 0; i < n_selvaggi; i++)
    {

    }
}

void cuoco()
{
    while (1)
    {
        // sem_wait(&vuoto);
        porzioni = n_porzioni;
        // sem_post(&p0ieno);
    }
}

void selvaggio()
{

}