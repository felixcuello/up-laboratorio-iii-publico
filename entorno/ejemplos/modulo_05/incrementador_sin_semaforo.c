#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARCHIVO "contador.txt"
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

    for (i = 0; i < ITERACIONES; i++) {
        valor = leer_contador();
        valor = valor + 1;
        usleep(1000);             /* 1 ms para forzar la race condition */
        escribir_contador(valor);
    }
    return 0;
}
