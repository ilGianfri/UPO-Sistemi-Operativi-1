#include <stdio.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "semfun.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int n_selvaggi = 0;
int shmid;
pid_t semid;

/* struct per le variabili da condividere con i vari processi */
typedef struct condivise
{
    int porzioni;                   /* Tiene conto del numero di porzioni ancora in pentola */
    int n_volte_riempie;            /* Tiene conto del numero di volte che il cuoco ha riempito la pentola */
    int n_porzioni;                 /* Numero porzioni massime che la pentola può contenere */
    int n_giri;                     /* Numero di volte cui un selvaggio mangia prima di terminare */
} var_condivise;

var_condivise *shared;

void cuoco();
void selvaggio();

int main(int argc, char *argv[])
{
    if (argc < 4) 
    {
        perror("Parametri non validi.\nUso: ./selvaggi #n_selvaggi# #n_porzioni# #n_giri#");
        return 1;
    }

    /*  
        Creazione dei semafori condivisi fra i processi

        IPC_PRIVATE usato come chiave garantisce che il vettore è nuovo e può essere 
        condiviso tra tutti i processi parenti (tramite id).

        Crea 3 semafori condivisi.
            - 1 per accedere alla variabile porzioni    (0)
            - 1 per indicare la pentola vuota           (1)
            - 1 per indicare la pentola piena           (2)

        0600 flag - permessi di lettura/scrittura al proprietario e nessun diritto agli altri

        Restituisce un "array" di semafori creati
    */

    semid = semget(IPC_PRIVATE, 3, 0600);
    if (semid == -1) 
    {
        perror("Errore semget\n");
        exit(1);
    }


    /*  
    
        Alloca la memoria condivisa fra i processi per le variabili condivise 
        
        IPC_PRIVATE indica che solo i processi figli potranno accedervi
        sizeof(var_condivise) indica quanto spazio deve essere allocato
        0600 permessi di lettura e scrittura al proprietario

    */
    shmid = shmget(IPC_PRIVATE, sizeof(var_condivise), 0600);
    if (shmid < 0)
        perror("Errore creazione memoria condivisa");

    /*
        Collega il segmento di memoria condivisa da shmid allo spazio di indirizzi del processo chiamante

        NULL poichè l'indirizzo viene scelto dal sistema
        0 = flag per comportamenti opzionali, 0 default

    */
    shared = (var_condivise *) shmat(shmid, NULL, 0);

    /* Salva i vari parametri passati nelle variabili */
    n_selvaggi = atoi(argv[1]);
    shared->n_porzioni = atoi(argv[2]);
    shared->n_giri = atoi(argv[3]);

    /* inizializza la pentola come piena */
    shared->porzioni = shared->n_porzioni;

    /*  Imposta i valori iniziali dei semafori */
    seminit(semid, 0, 1); /* MUTEX PORZIONI */
    seminit(semid, 1, 0); /* PENTOLA VUOTA */
    seminit(semid, 2, 0); /* PENTOLA PIENA */

    /* Creazione processo cuoco */
    pid_t pidcuoco = fork();
    if (pidcuoco == (pid_t)-1)
    {
        perror("fork cuoco fallita");
        return 1;
    }
    else if (pidcuoco == (pid_t)0)
        cuoco();

    /* Creazione processi selvaggi */
    for (int i = 0; i < n_selvaggi; i++)
    {
        pid_t selvid = fork();
        if (selvid == (pid_t)-1)
        {
            perror("fork selvaggio fallita");
            return 1;
        }
        else if (selvid == (pid_t)0)
            selvaggio(i + 1);
        else
        {
            /* Attende che il selvaggio abbia terminato */
            wait(NULL);
        }
        
    }

    /* Chiude il processo cuodo */
    kill(pidcuoco, SIGTERM);
    printf("\n\nLa pentola è stata riempita %d volte. Sono avanzate %d porzioni\n", shared->n_volte_riempie, shared->porzioni);
}

void cuoco()
{
    while (1)
    {
        /* Down del semaforo 'vuoto' */
        down(semid, 1);

        /* Se la pentola è vuota, la riempie */
        if (shared->porzioni <= 0)
        {
            /* Incremento porzioni - cuoco cucina */
            for (int i = 0; i < shared->n_porzioni; ++i)
                shared->porzioni++;

            /* Incrementa il numero di volte in cui la pentola è riempita */
            shared->n_volte_riempie++;
            printf("\nPentola riempita. Cuoco dorme\n\n");

            /* Up del semaforo 'pieno' */
            up(semid, 2);
        }
    }
}

void selvaggio(int n)
{
    for (int i = 0; i < shared->n_giri; ++i)
    {
        /* Accede alla variabile porzioni */
        down(semid, 0);

        printf("Selvaggio %d accede a porzioni\n", n);
        if (shared->porzioni == 0)
        {
            printf("Pentola vuota, sveglia il cuoco\n");

             /* Sveglia cuoco - semaforo 'vuoto' */
             up(semid, 1);
             /* Aspetta che il cuoco riempie - semaforo 'pieno' */
             down(semid, 2);
        }

        shared->porzioni--;
        printf("Selvaggio %d mangia una porzione, restano %d porzioni\n", n, shared->porzioni);
        /* Libera la variabile porzioni */
        up(semid, 0);  
    }

    /* Esecuzione selvaggio terminata, termina con codice 0 */
    exit(0);
}