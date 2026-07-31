#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

#define ARCHIVO "contador.txt"
#define SEMAFORO "/sem_contador"
#define ITERACIONES 100

int leer_contador(void)
{
    FILE *f;
    int valor;

    f = fopen(ARCHIVO, "r");
    if (f == NULL) {
        perror("fopen lectura");
        exit(EXIT_FAILURE);
    }
    if (fscanf(f, "%d", &valor) != 1) {
        valor = 0;
    }
    fclose(f);
    return valor;
}

void escribir_contador(int valor)
{
    FILE *f;

    f = fopen(ARCHIVO, "w");
    if (f == NULL) {
        perror("fopen escritura");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "%d\n", valor);
    fclose(f);
}

int main(void)
{
    int i;
    int valor;
    sem_t *sem;

    sem = sem_open(SEMAFORO, O_CREAT, 0600, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ITERACIONES; i++) {
        if (sem_wait(sem) == -1) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }

        /* INICIO de la sección crítica */
        valor = leer_contador();
        valor = valor + 1;
        usleep(1000);
        escribir_contador(valor);
        /* FIN de la sección crítica */

        if (sem_post(sem) == -1) {
            perror("sem_post");
            exit(EXIT_FAILURE);
        }
    }

    sem_close(sem);
    return 0;
}
