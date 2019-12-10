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

int n_selvaggi = 0, n_porzioni = 0, n_giri = 0;
pid_t semid;

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
            - 1 per accedere alla variabile porzioni (0)
            - 1 per indicare la pentola vuota   (1)
            - 1 per indicare la pentola piena   (2)

        0600 flag - permessi di lettura/scrittura al proprietario e nessun diritto agli altri

        Restituisce un "array" di semafori creati
    */

    semid = semget(IPC_PRIVATE, 3, 0600);
    if (semid == -1) 
    {
        perror("Errore semget\n");
        exit(1);
    }

    printf("ID semafori: %d\n",semid);    

    /*  Imposta i valori iniziali dei semafori */
    seminit(semid, 0, 1);
    seminit(semid, 1, 1);
    seminit(semid, 2, 0);

    /* Creazione processo cuoco */
    pid_t pidcuoco = fork();
    if (pidcuoco == (pid_t)-1)
    {
        perror("fork cuoco fallita");
        return 1;
    }
    else if (pidcuoco == (pid_t)0)
        cuoco();

    for (int i = 0; i < n_selvaggi; i++)
    {
        pid_t selvid = fork();
        if (selvid == (pid_t)-1)
        {
            perror("fork selvaggio fallita");
            return 1;
        }
        else if (selvid == (pid_t)0)
            selvaggio();
    }
}

void cuoco()
{
    while (1)
    {
        /*  Down del semaforo 'mutex' per accedere alla 
            variabile porzioni */
        down(semid, 0);
        printf("Cuoco accede alla variabile porzioni\n");
        /* Down del semaforo 'vuoto' */
        down(semid, 1);

        /* Incremento porzioni */
        porzioni = n_porzioni;
        n_volte_riempie++;
        printf("Pentola riempita\n");

        /* Up del semaforo 'pieno' */
        up(semid, 2);
        
        /* Up del semaforo 'mutex' */
        up(semid, 0);
        
        printf("Cuoco dorme\n");
    }
}

void selvaggio()
{
    for (int i = 0; i < n_giri; i++)
    {
        /* Accede alla variabile porzioni */
        down(semid, 0);
        printf("Selvaggio accede a porzioni\n");
        if (porzioni == 0)
        {
            printf("Pentola vuota, sveglia il cuoco\n");
            /* Sveglia lo stupido cuoco */
            up(semid, 1);
        }
        porzioni--;
        printf("Selvaggio mangia una porzione, restano %d porzioni\n", porzioni);
        /* Libera la variabile porzioni */
        up(semid, 0);
    }
}